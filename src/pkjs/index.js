var keys = require('message_keys');

const API_BASE_URL = "https://api.themeparks.wiki/preview/parks/";
const API_PARKS_KEYS = [
    "WaltDisneyWorldMagicKingdom",
    "WaltDisneyWorldEpcot",
    "WaltDisneyWorldHollywoodStudios",
    "WaltDisneyWorldAnimalKingdom",
    "DisneylandResortMagicKingdom",
    "DisneylandResortCaliforniaAdventure",
    "Efteling",
    "DisneylandParisWaltDisneyStudios",
    "DisneylandParisMagicKingdom",
    "HongKongDisneylandPark",
    "ShanghaiDisneylandPark",
    "TokyoDisneyland",
    "TokyoDisneySea",
    "UniversalStudios",
    "UniversalIslandsOfAdventure",
    "EuropaPark",
    "UniversalStudiosFlorida",
    "UniversalVolcanoBay"
];

function returnParkWaitTimes(attrTimesObject) {
    // Watch communication expects attraction names, wait times, and their 
    // status (string)
    // We only want entries of the type ATTRACTION and with "active" true
    // We also want to sort everything alphabetically by attraction name, too.

    // Sort the objects by the "name" key
    attrTimesObject.sort(function(a, b) {return a.name.localeCompare(b.name);})

    // Build the list of attraction names
    var payloadNames = [];
    // Build the list of attraction statuses
    var payloadStatuses = [];

    attrTimesObject.forEach(function (attraction) {
        // Verify that the entry is actually an attraction
        if (attraction.meta.type !== "ATTRACTION") return;

        // If it is, separate it out into the attractions lists
        payloadNames.push(attraction.name);
        // Sometimes the wait time can be null. In such a case, we send the
        // status message
        if (attraction.waitTime == null) {
            payloadStatuses.push(attraction.status);
        } else {
            var waitString = attraction.waitTime.toString() + " minute";
            if(attraction.waitTime != 1) {
                waitString += "s";
            }
            payloadStatuses.push(waitString);
        }
    });

    // Our lists have been created. Time to create the payload object
    var payloadDict = {};
    for (var i = 0; i < payloadNames.length; i++) {
        payloadDict[keys.i_attractionNameString + i] = payloadNames[i];
        payloadDict[keys.i_attractionStatus + i] = payloadStatuses[i];
    }

    payloadDict[keys.i_attractionCount] = payloadNames.length;

    // Send it.
    Pebble.sendAppMessage(payloadDict, function () {
        console.log('Message sent successfully: ' + JSON.stringify(payloadDict));
    }, function (e) {
        console.log('Message failed: ' + JSON.stringify(e));
    });
}

function getParkWaitTimes(parkID) {
    var method = 'GET';
    var url = API_BASE_URL + API_PARKS_KEYS[parkID] + "/waittime";

    var request = new XMLHttpRequest();

    request.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            // Parse the response
            try {
                var response = JSON.parse(this.responseText);
                console.log("Got a wait times response with " + response.length + " entries");
                returnParkWaitTimes(response);
            } catch (e) {
                console.log("Error parsing wait times response: " + e.name + " > " + e.message);
            }
        }
    }

    request.open(method, url);
    request.send();
}

function fixMissingNumbers(dict) {
    for (var k in keys) {
        if (Object.prototype.hasOwnProperty.call(keys, k)) {
            if (Object.prototype.hasOwnProperty.call(dict, k)) {
                dict[keys[k]] = dict[k]
            }
        }
    }
}

Pebble.addEventListener("ready", function (e) {
    // We're ready to roll!
    console.log("PebbleKit JS ready!");

    Pebble.addEventListener('appmessage', function (e) {
        // Get the dictionary from the message
        var data = e.payload;
        fixMissingNumbers(data);

        console.log("Got a request...");
        console.log(JSON.stringify(data));

        // Is this a parkID request?
        if (data[keys.o_parkID] !== undefined) {
            // It is! We need to get that information from the server
            console.log("Got a request to get the data for parkID " + data[keys.o_parkID]);
            getParkWaitTimes(data[keys.o_parkID]);
        }
    });
});
