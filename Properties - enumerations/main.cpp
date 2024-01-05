#include <stdio.h>      // C Standard Input/Output library.
#include <string>       // C++ standard string library
#include <regex>        // C++11 standard regex library

#include "XCamera.h"    // Xeneth SDK main header.

//////////////////////////////////////////////////////////////////////////
//
// The utility functions FindFirstPropertyRangePair and FindNextPropertyRangePair helps the user in parsing the
// property range string obtained using the XC_GetPropertyRangeE-function. This function contains a new 
// format for the possible enumeration entries which holds both the enumeration entry's key and display name.
//

typedef struct {
    std::string range;
    std::string::size_type pos;

    const char DELIM_PAIRS = ',';
    const char DELIM_KVP = '=';
} PropertyRangeCtx;


bool FindFirstPropertyRangePair(PropertyRangeCtx & ctx, std::string range, std::string & enumEntryName, std::string & enumEntryDisplayName);
bool FindNextPropertyRangePair(PropertyRangeCtx & ctx, std::string & enumEntryName, std::string & enumEntryDisplayName);

//////////////////////////////////////////////////////////////////////////
//
// Entry point
//

int main()
{
    /* 
     *  In this sample the BitShift is used to demonstrate the usage of the new 
     *  enumeration property functions. This property is available in the virtual
     *  camera interface to configure the image's pixel size generated by this camera.
     */
    std::string propertyName("BitShift");
    std::string cameraUrl("soft://0");

    printf("Opening connection to virtual camera '%s'\r\n", cameraUrl.c_str());
    XCHANDLE handle = XC_OpenCamera(cameraUrl.c_str());

    if (XC_IsInitialised(handle)) {
        std::string propertyRange;
        propertyRange.resize(1024);

        //////////////////////////////////////////////////////////////////////////
        // Retrieve enumeration property range

        ErrCode err = XC_GetPropertyRangeE(handle, propertyName.c_str(), &propertyRange[0], 1024);
        if (I_OK != err) {
            printf("Could not retrieve property range for '%s': %i\r\n", propertyName.c_str(), err);
            XC_CloseCamera(handle);
            return -1;
        }
        printf("Retrieved property range for '%s'.\r\nRange: %s\r\n", propertyName.c_str(), propertyRange.c_str());

        //////////////////////////////////////////////////////////////////////////
        // Parse enumeration property range using utility functions:
        // FindFirstPropertyRangePair and FindNextPropertyRangePair.

        unsigned int cnt = 0;
        PropertyRangeCtx ctx;
        std::string enumEntryName, enumEntryDisplayName;

        if (FindFirstPropertyRangePair(ctx, propertyRange, enumEntryName, enumEntryDisplayName)) {
            do {
                cnt++;
                printf("Entry #%i: %s = %s\r\n", cnt, enumEntryName.c_str(), enumEntryDisplayName.c_str());
            } while(FindNextPropertyRangePair(ctx, enumEntryName, enumEntryDisplayName));
        }

        //////////////////////////////////////////////////////////////////////////
        // Retrieve enumeration property value

        std::string propertyValue;
        propertyValue.resize(1024);
        err = XC_GetPropertyValueE(handle, propertyName.c_str(), &propertyValue[0], 1024);
        if (I_OK != err) {
            printf("Could not retrieve property value for '%s'\r\n", propertyName.c_str());
            XC_CloseCamera(handle);
            return -1;
        }
        printf("Retrieved property value for '%s': %s\r\n", propertyName.c_str(), propertyValue.c_str());

        //////////////////////////////////////////////////////////////////////////
        // Set enumeration property range value

        propertyValue = "camera16bit";
        err = XC_SetPropertyValueE(handle, propertyName.c_str(), propertyValue.c_str());
        if (I_OK != err) {
            printf("Could not set the new enumeration property value for '%s'\r\n", propertyName.c_str());
            XC_CloseCamera(handle);
            return -1;
        }
        printf("Set value for '%s' to '%s'\r\n", propertyName.c_str(), propertyValue.c_str());

    }
    else {
        printf("Camera initialization failed. Could not connect '%s'\r\n", cameraUrl.c_str());
    }

    XC_CloseCamera(handle);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//
//  Utility function implementations
//

std::string EntityDecode(std::string string_decode) {
    if (!string_decode.empty()) {
        // convert entities to their character equivalent
        // only printable ASCII characters are supported.
        std::regex entity_regex("&#([0-9]{1,3});");
        std::smatch entity_match;

        while (std::regex_search(string_decode, entity_match, entity_regex) && entity_match.ready()) {
            long entity_dec_value = strtol(entity_match[1].str().c_str(), nullptr, 10);
            if (isprint(entity_dec_value)) {
                string_decode.replace(
                    entity_match.position(), 
                    entity_match.length(), 1,
                    (char)entity_dec_value
                    );
            }
        }
    }

    return string_decode;
}

bool FindNextPropertyRangePair(PropertyRangeCtx & ctx, std::string & enumEntryName, std::string & enumEntryDisplayName) {
    if (ctx.pos == std::string::npos) return false;

    std::string::size_type pos_pairs(ctx.range.find(ctx.DELIM_PAIRS, ctx.pos));
    std::string pair(ctx.range.substr(ctx.pos, pos_pairs - ctx.pos));
    std::string::size_type pos_kvp = pair.find(ctx.DELIM_KVP);

    if (pos_kvp == std::string::npos) return false;
    enumEntryName = pair.substr(0, pos_kvp);
    enumEntryDisplayName = EntityDecode(pair.substr(pos_kvp + 1));

    if (pos_pairs != std::string::npos) pos_pairs++;
    ctx.pos = pos_pairs;

    return true;
}

bool FindFirstPropertyRangePair(PropertyRangeCtx & ctx, std::string range, std::string & enumEntryName, std::string & enumEntryDisplayName) {
    // first make sure the context is in its reset state
    ctx.range = range;
    ctx.pos = 0;

    // when reset, find the first property range pair
    return FindNextPropertyRangePair(ctx, enumEntryName, enumEntryDisplayName);
}
