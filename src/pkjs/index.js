var keys = require('message_keys');
var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

const API_BASE_URL = "https://api.themeparks.wiki/v1/entity/";

function formatAttractionData(attraction, showShowTimes, showValidDataOnly) {
    // If it's not an attraction or a show, return
    if (!(attraction.entityType == "ATTRACTION" ||
        (attraction.entityType == "SHOW" && showShowTimes))) return;

    attractionResult = [
        attraction.name,
        attraction.status || ""
    ];

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
        var hasQueueData = false;

        if(queueData.BOARDING_GROUP != undefined) {
            const bgData = queueData.BOARDING_GROUP;
            if(bgData.currentGroupStart != null && bgData.currentGroupEnd != null) {
                attractionResult[1] = "Groups " + bgData.currentGroupStart.toString() + "-" + bgData.currentGroupEnd.toString();
                hasQueueData = true;
            }
        }

        if(!hasQueueData && queueData.STANDBY != null && queueData.STANDBY.waitTime != null) {
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
    console.log("Beginning to prepare wait times for return to pebble");
    var liveData = attrTimesObject.liveData;
    console.log("Collected live data. Sorting " + liveData.length + " attractions");
    var counter = 0;
    liveData.forEach(function (attraction) {
        console.log(counter + ": " + attraction.name);
        counter += 1;
    });
    
    liveData.sort(function (a, b) { 
        return a.name.toString().localeCompare(b.name.toString()); 
    })

    console.log("Sorted livedata by name. Moving on")

    // Build the list of attraction names
    var payloadNames = [];
    // Build the list of attraction statuses
    var payloadStatuses = [];

    liveData.forEach(function (attraction) {
        console.log("Formatting data for " + attraction.name);
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
    var url = API_BASE_URL + parkID + "/live";

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
                console.log(e.stack);
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


var Destinations = require('./destinations');
var destinations = new Destinations();

var clayLookup = {}

function launchClayWithDynamicConfig() {
    // get our new config data from destinations
    const dest_data = destinations.cache_data.destinations;
    console.log("Asked to launch clay with data from destinations: " + destinations.cache_data);
    console.log(typeof destinations.cache_data)

    clayLookup = {};
    // Build our dynamic parks data using the data from dest_data
    const out_entries = []
    const out_config = {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Parks"
            },
            {
                "type": "text",
                "defaultValue": "Select the parks you'd like to see on your watch."
            },
            {
                "type": "text",
                "defaultValue": "Data last fetched " + destinations.getCacheAgeInDays().toFixed(0) + " day(s) ago"
            }
        ]
    }

    const lonely_parks = []

    dest_data.forEach(element => {
        const dest_parks = []

        if(element.parks.length == 1) {
            lonely_parks.push([element.parks[0].name, element.parks[0].id, element.name]);
        } else {
            element.parks.forEach(park => {
                dest_parks.push([park.name, park.id, element.name]);
            });

            dest_parks.sort((a, b) => a[0].localeCompare(b[0]));

            clayLookup[element.id] = dest_parks;

            out_entries.push({
                "type": "checkboxgroup",
                "messageKey": "pig_" + element.id,
                "label": element.name,
                "options": dest_parks.map(item => item[0]),
                "group": "park_input",
            });
        }
    });

    out_entries.sort((a, b) => a.label.localeCompare(b.label));

    lonely_parks.sort((a, b) => a[0].localeCompare(b[0]))

    clayLookup["lonely"] = lonely_parks;
    out_entries.push({
        "type": "checkboxgroup",
        "messageKey": "pig_" + "lonely",
        "label": "Destination Parks",
        "group": "park_input",
        "options": lonely_parks.map(item => item[0])
    })


    out_config.items.push(...out_entries)

    const new_config = JSON.parse(JSON.stringify(clayConfig));
    new_config[2] = out_config;

    clay.config = new_config;
    // console.log(clay.generateUrl());
    Pebble.openURL(clay.generateUrl());
}

Pebble.addEventListener("ready", function (e) {
    // We're ready to roll!
    console.log("PebbleKit JS ready!");

    // Let the app know we're ready to process messages
    var payload = {};
    payload[keys._ready] = 1;
    Pebble.sendAppMessage(payload, function () { });

    console.log("Destinations is currently fresh? " + destinations.hasFreshDestinations());
    console.log(destinations.cache_data);
});

Pebble.addEventListener('showConfiguration', function(e) {
    // Run another web request to fetch the destinations endpoint. Store that locally,
    // and prepare it for Clay.
    const hasFreshDestinations = destinations.hasFreshDestinations();
    console.log("Destinations is currently fresh? " + hasFreshDestinations);
    console.log(destinations.cache_data);

    // Mark destinations for a refresh
    if(!hasFreshDestinations) destinations.refreshCache();

    // Load anyway - default destinations data as a fallback
    launchClayWithDynamicConfig();
})

Pebble.addEventListener('webviewclosed', function(e) {
    if(e && !e.response) return;

    // We need to somehow transform the arbitrary true/false responses into
    // usable stuff for our watchapp...

    // I think we might be able to do so in an interesting way. Let's check
    // console.log(JSON.stringify(e.response));
    const handled_response = JSON.parse(e.response);
    console.log(e.response);
    const filtered_response = {}

    // Okay. We've baked in our data in the "pig" message tags. If we
    // strip the "pig_" part out of the keys, we can reverse-engineer which
    // of the parks the user wants...
    var desired_park_names = [];
    var desired_park_ids = [];
    var desired_park_destination = [];

    var desired_destination_names = [];
    const inbound_keys = Object.keys(handled_response);
    inbound_keys.forEach((in_key) => {
        // Ignore things that don't contain our special tag

        filtered_response[in_key] = handled_response[in_key];

        if(!(in_key.includes("pig_"))) {
            filtered_response[in_key] = handled_response[in_key];
        }

        const inbound_value = handled_response[in_key]["value"];

        // Strip the index from the in_key
        const inbound_index = in_key.substring(4);

        // Lookup the dict using our index values
        const lookup_values = clayLookup[inbound_index]

        // Include the lookup_values only if the flag is true
        for (let i = 0; i < inbound_value.length; i++) {
            const park_true = inbound_value[i];
            if(!park_true) continue;

            const lookup_data = lookup_values[i];
            const destination_name = lookup_data[2];
            let destination_index = desired_destination_names.indexOf(destination_name);
            console.log("looking at " + destination_index + " in " + JSON.stringify(desired_destination_names) + " for " + destination_name);
            if(destination_index == -1) {
                destination_index = desired_destination_names.push(destination_name) - 1;
                console.log("couldn't find it. pushing, now " + JSON.stringify(desired_destination_names) + " and I think it's at " + destination_index);
            }

            desired_park_names.push(lookup_data[0]);
            desired_park_ids.push(lookup_data[1]);
            desired_park_destination.push(destination_index);
        }
    });

    // Crop down the lists to, at most, 32 parks.
    // This is per the limit on the messagekey.
    var net_park_count = desired_park_names.length;
    const max_park_count = 32;
    if(net_park_count > max_park_count) {
        desired_park_names = desired_park_names.slice(0, max_park_count);
        desired_park_ids   = desired_park_ids.slice(0, max_park_count);
        desired_park_destination = desired_park_destination.slice(0, max_park_count);

        // TODO: Properly handle stripping back destination values
        desired_destination_names = desired_destination_names.slice(0, max_park_count);

        console.log("WARNING: Net park count of " + net_park_count + " was too long (longer than " + max_park_count + "). Trimming.");
        
        net_park_count = 32;
    }

    var net_dest_count = desired_destination_names.length;

    filtered_response["c_newpark_names"]     = {"value": desired_park_names};
    filtered_response["c_newpark_ids"]       = {"value": desired_park_ids};
    filtered_response["c_newpark_destids"]   = {"value": desired_park_destination};
    filtered_response["c_newpark_destnames"] = {"value": desired_destination_names};
    filtered_response["c_newpark_count"]     = {"value": net_park_count};
    filtered_response["c_newpark_destcount"] = {"value": net_dest_count};

    // console.log(JSON.stringify(filtered_response));

    // This transforms the clay settings into stuff that the Pebble can handle
    // BUT! It also stores the data into localstorage.
    //
    // This means that if we want to store our custom PIG stuff in there, we've
    // got to handle it on the config side in a custom function on load...
    //
    // Gross..
    var dict = clay.getSettings(JSON.stringify(filtered_response));
    console.log(JSON.stringify(dict));
    Pebble.sendAppMessage(dict, function(e) {
        console.log('Sent config data to Pebble');
    }, function(e) {
        console.log('Failed to send config data!');
        console.log(JSON.stringify(e));
    })
})

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
        getParkWaitTimes(data[keys.o_parkID], true, false);
    }
});
