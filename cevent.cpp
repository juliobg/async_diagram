#include "cevent.h"

std::regex CEvent::iRegEx("^.*\\^\\^\\^(.*):\\|(.*):\\|(.*):\\|.*->(.*):\\|(.*):\\|\\s*([-+]?(\\d*[.])?\\d+).*$");

std::regex CEvent::iRegExEvent("^.*\\^\\^\\^(.*):\\|(.*):\\|(.*):\\|\\s*([-+]?(\\d*[.])?\\d+).*$");


//^^^ Event fired :| PlacesClient :| Connector registered. :| 5526378525364
