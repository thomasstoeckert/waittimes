var keys = require('message_keys');
var Clay = require('pebble-clay');

const API_BASE_URL = "https://api.themeparks.wiki/v1/entity/";
const API_PARKS_KEYS = [
    // --- CALIFORNIA PARKS --- //
    // Disneyland Resort
    "7340550b-c14d-4def-80bb-acdb51d49a66", // Disneyland Park
    "832fcd51-ea19-4e77-85c7-75d5843b127c", // Disney's California Adventure
    // Other
    "bdf9b533-144c-4b78-aa2f-5173c5ce5e85", // California's Great America
    "0a6123bb-1e8c-4b18-a2d3-2696cf2451f5", // Knott's Berry Farm
    "75122979-ddea-414d-b633-6b09042a227c", // SeaWorld San Diego
    "bc4005c5-8c7e-41d7-b349-cdddf1796427", // Universal Studios (Hollywood)

    // --- ORLANDO PARKS --- //
    // Walt Disney World
    "75ea578a-adc8-4116-a54d-dccb60765ef9", // Magic Kingdom
    "47f90d2c-e191-4239-a466-5892ef59a88b", // Epcot
    "288747d1-8b4f-4a64-867e-ea7c9b27bad8", // Hollywood Studios
    "1c84a229-8862-4648-9c71-378ddd2c7693", // Animal Kingdom
    "b070cbc5-feaa-4b87-a8c1-f94cca037a18", // Typhoon Lagoon
    "ead53ea5-22e5-4095-9a83-8c29300d7c63", // Blizzard Beach
    // Universal Orlando
    "eb3f4560-2383-4a36-9152-6b3e5ed6bc57", // Universal Studios (Florida)
    "267615cc-8943-4c2a-ae2c-5da728ca591f", // Universal Islands of Adventure
    "fe78a026-b91b-470c-b906-9d2266b692da", // Volcano Bay
    // Sea World
    "27d64dee-d85e-48dc-ad6d-8077445cd946", // SeaWorld Orlando
    "9e2867f8-68eb-454f-b367-0ed0fd72d72a", // Aquatica Orlando
    // Other
    "bb285952-7e52-4a07-a312-d0a1ed91a9ac", // Legoland Florida
    "fc40c99a-be0a-42f4-a483-1e939db275c2", // Busch Gardens Tampa Bay

    // --- US National Parks --- //
    "98f634cd-c388-439c-b309-960f9475b84d", // Busch Gardens Williamsburg
    "66f5d97a-a530-40bf-a712-a6317c96b06d", // Canada's Wonderland
    "24cdcaa8-0500-4340-9725-992865eb18d6", // Carowinds
    "c8299e1a-0098-4677-8ead-dd0da204f8dc", // Cedar Point
    "7502308a-de08-41a3-b997-961f8275ab3c", // Dollywood
    "19d7f29b-e2e7-4c95-bd12-2d4e37d14ccf", // Dorney Park
    "0f044655-cd94-4bb8-a8e3-c789f4eca787", // Hersheypark
    "95162318-b955-4b7e-b601-a99033aa0279", // Kings Dominion
    "a0df8d87-7f72-4545-a58d-eb8aa76f914b", // Kings Island
    "e9805d65-edad-4700-8942-946e6a2b4784", // Michigan's Adventure
    "dd0e159a-4e4b-48e5-8949-353794ef2ecb", // SeaWorld San Antonio
    "d21fac4f-1099-4461-849c-0f8e0d6e85a6", // Silver Dollar City
    "1989dca9-c8d3-43b8-b0dd-e5575f692b95", // Valleyfair
    "bb731eae-7bd3-4713-bd7b-89d79b031743", // Worlds of Fun

    // --- UK Parks --- //
    "0d8ea921-37b1-4a9a-b8ef-5b45afea847b", // Alton Towers
    "ae959d1f-9fcc-4aab-8063-71e641fa57f4", // Chessington World of Adventures
    "a4f71074-e616-4de4-9278-72fdecbdc995", // Legoland Windsor
    "b08d9272-d070-4580-9fcd-375270b191a7", // Thorpe Park

    // --- EU Parks --- //
    // Germany
    "639738d3-9574-4f60-ab5b-4c392901320b", // Europa Park
    "58392c29-d79d-49e4-9c35-0100d417d24e", // Rulantica
    "abb67808-61e3-49ef-996c-1b97ed64fac6", // Phantasialand
    "66e12a41-3a09-40cd-8f55-8d335d9d7d93", // Heide Park
    // Holland
    "18635b3e-fa23-4284-89dd-9fcd0aaa9c9c", // Walibi Holland

    // France
    "dae968d5-630d-4719-8b06-3d107e944401", // Disneyland Paris
    "ca888437-ebb4-4d50-aed2-d227f7096968", // Walt Disney Studios Park
    "9e938687-fd99-46f3-986a-1878210378f8", // Parc Asterix

    // Belgium
    "164f3ee7-5fd7-47ac-addc-40b5e3e2b144", // Bellewaerde
    "f0ea9b9c-1ccb-4860-bfe6-b5aea7e4db2b", // Plopsaland De Panne

    // Brussels
    "3ad67aac-0b97-4961-8aa8-c3b8d2350873", // Holiday Park

    // Sweden
    "93142d7e-024a-4877-9c72-f8e904a37c0c", // Liseberg

    // Netherlands
    "30713cf6-69a9-47c9-a505-52bb965f01be", // Efteling
    "f4bd1a23-44f0-444b-a91c-8d24f6ec5b1f", // Attractiepark Toverland

    // Italy
    "043211c0-76f2-4456-89f8-4001be01018d", // Gardaland

    // Spain
    "32608bdc-b3fa-478e-a8c0-9dde197a4212", // PortAventura Park
    "d06d91b8-7702-42c3-a8af-7d0161d471bf", // Ferrari Land
    "f9c2e042-8604-4fe8-9909-e8f95f0942f5", // Aquatic Park

    // --- Asia Parks --- //
    "bd0eb47b-2f02-4d4d-90fa-cb3a68988e3b", // Hong Kong Disneyland
    "ddc4357c-c148-4b36-9888-07894fe75e83", // Shanghai Disneyland
    "3cc919f1-d16d-43e0-8c3f-1dd269bd1a42", // Tokyo Disneyland
    "67b290d5-3478-4f23-b601-2f8fb71ba803"  // Tokyo DisneySea
];

function formatAttractionData(attraction, showShowTimes, showValidDataOnly) {
    // If it's not an attraction or a show, return
    if (!(attraction.entityType == "ATTRACTION" ||
        (attraction.entityType == "SHOW" && showShowTimes))) return;

    attractionResult = [];
    attractionResult[0] = attraction.name;
    attractionResult[1] = attraction.status || "";

    var hasValidData = false;

    // If it is a show, get the next showtime and localize it
    if (attraction.entityType == "SHOW") {
        const showtimeData = attraction.showtimes;
        if (showtimeData != undefined) {
            var currentShow = null;
            var nextShow = null;
            var showsLeft = showtimeData.length;
            
            const now = new Date();

            const numShows = showtimeData.length;
            for (var i = 0; i < numShows; i++) {
                const show = showtimeData[i];
                const startTimeParsed = new Date(show.startTime);
                const endTimeParsed = new Date(show.endTime);

                // Decrement the amount of shows left in the day, minus one
                showsLeft--;

                if(now < startTimeParsed) {
                    // This show starts after now. It is our next show.
                    nextShow = show;
                    break;
                }

                if(now >= startTimeParsed &&  now < endTimeParsed) {
                    // This show has started, but ends later. It is our
                    // current show.
                    currentShow = show;
                }
            }

            if(!nextShow && !currentShow) {
                // No show running, no more today
                // Display text
                attractionResult[1] = "No More Shows";
            } else if (!nextShow && currentShow) {
                // Show running, no more today
                // Display end time of this show
                attractionResult[1] = "Ends " + (new Date(currentShow.endTime)).toLocaleTimeString('default', {timeStyle: 'short'});
                hasValidData = true;
            } else if (nextShow) {
                // More shows later. 
                // Display start time of the next show
                attractionResult[1] = (new Date(nextShow.startTime)).toLocaleTimeString('default', {timeStyle:'short'});
                if(showsLeft > 1) attractionResult[1] += " + " + showsLeft.toString() + " more";
                hasValidData = true;
            }
        }
    } else if (attraction.queue != undefined) {
        // It is an attraction. Do our attraction things.
        const queueData = attraction.queue;

        // If there's virtual line / boarding group data, display that first
        if (queueData.BOARDING_GROUP != undefined) {
            const bgData = queueData.BOARDING_GROUP;
            attractionResult[1] = "Groups " + bgData.currentGroupStart.toString() + "-" + bgData.currentGroupEnd.toString();
            hasValidData = true;
        } else if(queueData.STANDBY != null && queueData.STANDBY.waitTime != null) {
            // If there be no standby data, try to throw the status up there
            const standbyData = queueData.STANDBY;
            var waitString = standbyData.waitTime.toString() + " minute";
            if (standbyData.waitTime != 1) {
                // Pluralize it!
                waitString += "s";
            }
            hasValidData = true;
            attractionResult[1] = waitString;
            hasValidData = true;
        }
    }
    if(!hasValidData && !showValidDataOnly) return;
    return attractionResult;
}

function returnParkWaitTimes(attrTimesObject, showShowTimes, showValidDataOnly) {
    // Watch communication expects attraction names, wait times, and their 
    // status (string)
    // We only want entries of the type ATTRACTION and with "active" true
    // We also want to sort everything alphabetically by attraction name, too.
    
    // Sort the objects by the "name" key
    var liveData = attrTimesObject.liveData;
    liveData.sort(function (a, b) { return a.name.localeCompare(b.name); })

    // Build the list of attraction names
    var payloadNames = [];
    // Build the list of attraction statuses
    var payloadStatuses = [];

    liveData.forEach(function (attraction) {
        result = formatAttractionData(attraction, showShowTimes, showValidDataOnly);
        if(result != null) {
            payloadNames.push(result[0]);
            payloadStatuses.push(result[1]);
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
        console.log('Message sent successfully');
    }, function (e) {
        console.log('Message failed: ' + JSON.stringify(e));
    });
}

function getParkWaitTimes(parkID, showShowTimes, showValidDataOnly) {
    var method = 'GET';
    var url = API_BASE_URL + API_PARKS_KEYS[parkID] + "/live";

    var request = new XMLHttpRequest();

    request.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            // Parse the response
            try {
                var response = JSON.parse(this.responseText);
                console.log("Got a wait times response with " + Object.keys(response).length + " entries");
                returnParkWaitTimes(response, showShowTimes, showValidDataOnly);
            } catch (e) {
                console.log("Error parsing wait times response: " + e.name + " > " + e.message);
            }
        }
    }

    request.onerror = function () {
        // Something wrong happened.
        var errorPayload = {};
        errorPayload[keys.i_connectionError] = 1;
        Pebble.sendAppMessage(errorPayload, function () {
            console.log("Error message sent to client");
        }, function (e) {
            console.log('Message failed: ' + JSON.stringify(e));
        });
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

var clayConfig = require('./config');
var clay = new Clay(clayConfig);

Pebble.addEventListener("ready", function (e) {
    // We're ready to roll!
    console.log("PebbleKit JS ready!");

    // Let the app know we're ready to process messages
    var payload = {};
    payload[keys._ready] = 1;
    Pebble.sendAppMessage(payload, function () { });
});

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
        getParkWaitTimes(data[keys.o_parkID], data[keys.c_showShowTimes], data[keys.c_showValidDataOnly]);
    }
});
