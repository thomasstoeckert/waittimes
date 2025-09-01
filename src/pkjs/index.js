var keys = require('message_keys');
var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

const AttractionsAPI = require('./attractions');
const attractions = new AttractionsAPI();

function handleWaitTimeResponse(response_data)
{
    const structured_data = attractions.reformatRawAttractionData(response_data);
    console.log("-------- RAW DATA -----------")
    console.log("\n" + JSON.stringify(structured_data) + "\n");
    // Get our settings from Clay
    const clay_data = JSON.parse(localStorage.getItem('clay-settings'));
    const sorted_and_filtered_data = attractions.sortAndFilterCleanAttractionData(structured_data, clay_data);
    
    console.log("-------- SORTED AND FILTERED DATA -----------")
    console.log("\n" + JSON.stringify(sorted_and_filtered_data) + "\n");

    const data_package = attractions.generateAttractionDataPackage(sorted_and_filtered_data);

    console.log("-------- Packaged Data -----------")
    console.log("\n" + JSON.stringify(data_package) + "\n");

    Pebble.sendAppMessage(data_package, function () {
        console.log("HWTR Message sent successfully");
    }, function (e) {
        console.log('HWTR Message Failed: ' + JSON.stringify(e));
    })
}

function fireWaitTimesRequest(park_id) {
    console.log(JSON.stringify(attractions));
    console.log(attractions);
    console.log(JSON.stringify(AttractionsAPI));
    attractions.fetchRawAttractionsForPark(park_id, handleWaitTimeResponse);
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
            // If the key isn't a pig key, pass it on and continue
            filtered_response[in_key] = handled_response[in_key];
            return
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
        fireWaitTimesRequest(data[keys.o_parkID]);
    }
});
