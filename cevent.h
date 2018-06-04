#ifndef CEVENT_H
#define CEVENT_H

#include <map>
#include <string>
#include <regex>

class CEvent;
typedef double TTimeStamp;
typedef std::shared_ptr<CEvent> TPEvent;

// trim from left
inline std::string ltrim(std::string s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from right
inline std::string rtrim(std::string s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from left & right
inline std::string trim(std::string s, const char* t = " \t\n\r\f\v")
{
    return ltrim(rtrim(s, t), t);
}

class CEvent
{
public:

    static std::regex iRegEx;
    static std::regex iRegExEvent;

    static TPEvent ReadLine(const std::string& aString)
    {
        std::smatch eventmatch;
        if (std::regex_match(aString, eventmatch, iRegEx)) {
            return std::make_shared<CEvent>(std::stod(eventmatch[6]), trim(eventmatch[4]), trim(eventmatch[3]), EStateChange);
        }
        else if (std::regex_match(aString, eventmatch, iRegExEvent) && eventmatch[1].str()==" Event fired ") {
            return std::make_shared<CEvent>(std::stod(eventmatch[4]), trim(eventmatch[3]), trim(eventmatch[2]), EEventFired);
        }

        return NULL;
    }

    enum EType
    {
        EStateChange,
        EEventFired
    };

    CEvent (TTimeStamp aTime, std::string aName, std::string aObject, EType aType)
        : iBegin(aTime), iName(aName), iObject(aObject), iType(aType) {}
    TTimeStamp iBegin;
    std::string iName;
    std::string iObject;
    EType iType;

};

class IEventCollectionObserver
{
public:
    virtual void NewElementAdded(const CEvent* aEvent) = 0;

};

class CEventCollection
{
private:

    std::vector<CEvent> iEvents;
    std::vector<IEventCollectionObserver*> iObservers;

public:
    void AddEvent(const CEvent& aEvent)
    {
        for (auto observer : iObservers) {
            observer->NewElementAdded(&aEvent);
        }

        iEvents.push_back(aEvent);
    }

    void AddObserver(IEventCollectionObserver* aObserver)
    {
        iObservers.push_back(aObserver);
    }
};



#endif // CEVENT_H
