const textCleaner = require('./text-cleaner');
const keys = require('message_keys');

function AttractionsAPI() {
    this.API_BASE_URL = "https://api.themeparks.wiki/v1/entity/";
}

// Step 1: Collect attractions
// Step 2: Re-format attractions data for internal use
// Step 3: Filter, sort attractions
// Step 4: Generate status messages
// Step 5: Prepare data for transmission

// Step 1: Collect raw attraction data
AttractionsAPI.prototype.fetchRawAttractionsForPark = function(park_id, callback) {
    const method = "GET";
    const url = this.API_BASE_URL + park_id + "/live";
    const request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if(this.readyState != 4) return;

        if(this.status == 200) {
            // yay!
            try {
                const response = JSON.parse(this.responseText);
                const livedata = response.liveData;
                console.log("Got raw attraction data back with " + livedata.length + " attractions");
                callback(livedata);
            } catch (e) {
                // D'aw. TODO: Send error
                console.log("Error while hitting attractions endpoint!");
                console.log(e.name);
                console.log(e.message);
                console.log(e.stack);
            }
        } else {
            // D'aw. TODO: Display Error
            console.log("Something went wrong with the response code :(");
            console.log(this.status);
            console.log(this.responseText);
        }
    }

    request.onerror = function() {
        // TODO: Display Error
        console.log("Something went wrong with the request itself :(");
    }

    request.open(method, url);
    request.send();
}

AttractionsAPI.prototype.reformatRawAttractionData = function(live_data) {
    // For each attraction in liveData
    // Transform it into the following structure:
    const out_data = [];
    /* 
    {
        "name": <ATTR NAME>,
        "type": <type>,
        "standby_wait": <STANDBY WAIT>,
        "next_showtimes": [<next showtime>, <following showtime>, etc.],
        "boarding_group": <min group> <max group>,
        "status": <STATUS STRING>
    }
    */
    live_data.forEach(raw_attraction => {
        const clean_attraction = {};
        // Clean the attraction name, too. Remove weird characters, trim
        // to max character length
        clean_attraction["name"]   = textCleaner(raw_attraction.name, 20);

        // Attraction status / type are pre-set by the API - not from the parks
        // themselves. This means that we don't have to worry about weird 
        // characters or too-long characters.
        //
        // We also don't display them directly on the watch.
        clean_attraction["type"]   = raw_attraction.entityType;
        clean_attraction["status"] = raw_attraction.status;

        // Standby wait!
        // Some things have them! Some don't! Let's see if it has it
        clean_attraction["standby_wait"] = null;
        if("queue" in raw_attraction &&
            "STANDBY" in raw_attraction.queue &&
            "waitTime" in raw_attraction.queue.STANDBY)
            clean_attraction["standby_wait"] = raw_attraction.queue.STANDBY.waitTime;
        
        // Showtimes! Filter out showtimes that have passed, but keep all that
        // haven't started yet.
        // 
        // Also include the current showtime, if it hasn't ended yet
        clean_attraction["next_showtimes"] = [];
        if("showtimes" in raw_attraction) {
            const now = Date.now();
            const raw_showtimes = raw_attraction.showtimes;
            const processed_showtimes = [];
            raw_showtimes.forEach(raw_showtime => {
                // See if this showtime has started
                const show_startime = new Date(raw_showtime.startTime);
                const show_endtime = new Date(raw_showtime.endTime);

                // If the show's already ended, return
                if(now > show_endtime) return;
                
                // Otherwise, use it.
                processed_showtimes.push([
                    show_startime, show_endtime
                ]);
            });
            // We only want to hold on to "in-progress" shows 
            // if it's the last one of the bunch. Otherwise, it's
            // confusing.
            //
            // So, if we have more than one upcoming showtime,
            // we check the first one in the list to see if it's
            // already started. If it has, we remove it.
            if(processed_showtimes.length > 1)
            {
                const first_showtime = processed_showtimes[0][0];
                if(first_showtime < now)
                    processed_showtimes.shift()
            }
            clean_attraction["next_showtimes"] = processed_showtimes;
        }

        // Boarding Groups! Grab the first/last that are currently able to be
        // redeemed
        clean_attraction["boarding_groups"] = [];
        if("queue" in raw_attraction &&
            "BOARDING_GROUP" in raw_attraction.queue) {
            const bg_startgroup = raw_attraction.queue.BOARDING_GROUP.currentGroupStart;
            const bg_endgroup   = raw_attraction.queue.BOARDING_GROUP.currentGroupEnd;
            if(bg_startgroup && bg_endgroup)
                clean_attraction["boarding_groups"] = [bg_startgroup, bg_endgroup];
        }

        out_data.push(clean_attraction);
    });

    return out_data;
}

AttractionsAPI.prototype.sortAndFilterCleanAttractionData = function(clean_data, settings) {
    // For each attraction, decide if we want it or not.
    // Afterwards, sort the lot.
    //
    // Settings block has the following things:
    // {
    //   "c_typeVisibility": <bool>[2],
    //   "c_hideClosed": <bool>,
    //   "c_sortOrder": <string>,
    // }
    // 
    // Sort and filter on the phone because we want to send as little data to
    // the pebble as we can :)

    const filtered_list = [];
    
    clean_data.forEach(clean_venue => {
        // If the attraction doesn't align with our type visibility settings,
        // skip it
        if(clean_venue["type"] == "ATTRACTION" && 
            !settings["c_typeVisibility"][0]) return;

        if(clean_venue["type"] == "SHOW" && 
            !settings["c_typeVisibility"][1]) return;
        
        if(clean_venue["type"] == "RESTAURANT" &&
            !settings["c_typeVisibility"][2]) return;
        
        // If it has no wait times...
        if(clean_venue["standby_wait"] == null &&
            // And no upcoming showtimes
            clean_venue["next_showtimes"].length <= 0 &&
            // And no boarding groups
            clean_venue["boarding_groups"].length <= 0 &&
            // And the user doesn't want to see it
            settings["c_hideClosed"]
            // Don't add it.
        ) return;

        filtered_list.push(clean_venue);
    });

    console.log("Filtered list down from " + clean_data.length + " to " + filtered_list.length);

    // Defining the sorting functions for comparison
    function sort_alpha_increase (a, b) {
        return a["name"].localeCompare(b["name"]);
    }
    function sort_alpha_decrease (a, b) {return -sort_alpha_increase(a, b)};

    function sort_wait (a, b, flip) {
        // If we don't have two wait times, sort by name
        if(a["standby_wait"] == null && b["standby_wait"] == null)
            return sort_alpha_increase(a, b);

        // If we have a wait, that one wins by default
        if(a["standby_wait"] == null)
            return 1;

        if(b["standby_wait"] == null)
            return -1;

        // Otherwise, actually compare wait times
        return (a["standby_wait"] - b["standby_wait"]) * flip;
    }

    const now = Date.now();
    function sort_shows (a, b, flip) {
        const a_has_shows = (a["next_showtimes"].length > 0);
        const b_has_shows = (b["next_showtimes"].length > 0);
        // If we don't have showtimes, sort by name
        if(!a_has_shows && !b_has_shows)
            return sort_alpha_increase(a, b);

        // If a venue doesn't have shows, the other wins by default
        if(!a_has_shows)
            return 1;

        if(!b_has_shows)
            return -1;

        // Actually comapre showtimes
        var a_start = a["next_showtimes"][0][0];
        var b_start = b["next_showtimes"][0][0];

        if(a_start < now)
            // Use a_end instead
            a_start = a["next_showtimes"][0][1];
        if(b_start < now)
            b_start = b["next_showtimes"][0][1];

        return (a_start - b_start) * flip;
    }

    function sort_smart (a, b) {
        // Sort (has wait time)
        // Sort (has boarding group)
        // Sort (has show time)
        // All others
        // Subsort by name

        // So, if one has a wait time, the other doesn't, prioritize the wait time
        // If one has a show time, the other doesn't, prioritize the show time,
        // If wait / show time is equal, sort by name
        const a_has_shows = (a["next_showtimes"].length > 0);
        const a_has_waits = a["standby_wait"] != null
        const a_has_bg    = a["boarding_groups"].length > 0;
        const b_has_shows = (b["next_showtimes"].length > 0)
        const b_has_waits = b["standby_wait"] != null
        const b_has_bg    = b["boarding_groups"].length > 0;

        if(a_has_waits && b_has_waits) {
            // Handle case where names is same
            if(a["standby_wait"] == b["standby_wait"])
                return a["name"].localeCompare(b["name"]);

            // Sort waittimes
            return (b["standby_wait"] - a["standby_wait"]);
        }

        // If one has a wait, but the other doesn't
        if(a_has_waits) return -1;
        if(b_has_waits) return 1;

        // Check BGs.
        if(a_has_bg && b_has_bg) {
            // Sort by name
            return a["name"].localeCompare(b["name"]);
        }

        if(a_has_bg) return -1;
        if(b_has_bg) return 1;


        // Check showtimes
        if(a_has_shows && b_has_shows) {
            // Handle case where next showtime is the same
            if(a["next_showtimes"][0] == b["next_showtimes"][0])
                return a["name"].localeCompare(b["name"]);

            // Otherwise, sort showtimes
            return (a["next_showtimes"][0][0] - b["next_showtimes"][0][0]);
        }

        // If one has a show, but the other doesn't
        if(a_has_shows) return -1;
        if(b_has_shows) return 1;

        // Neither shows, bgs, or waittimes. Sort by name
        return a["name"].localeCompare(b["name"]);
    }

    const sort_lut = {
        "alpha_increase": sort_alpha_increase,
        "alpha_decrease": sort_alpha_decrease,
        "wait_increase": (a, b) => sort_wait(a, b, 1),
        "wait_decrease": (a, b) => sort_wait(a, b, -1),
        "shows_increase": (a, b) => sort_shows(a, b, 1),
        "shows_decrease": (a, b) => sort_shows(a, b, -1),
        "sort_smart": sort_smart
    }

    const selected_sort = sort_lut[settings["c_sortOrder"]];

    filtered_list.sort(selected_sort);

    // Finally, trim the list to the maximum allotted for this data structure
    // (128)
    if(filtered_list.length > 128)
        filtered_list = filtered_list.slice(0, 128);

    return filtered_list;
}

AttractionsAPI.prototype.generateAttractionDataPackage = function(clean_sorted_data) {
    // Build a transmittable attractions data package (arr_name, arr_statusstrings, count)
    // for sending to the watch by someone else

    // Restated, take the data format of 
    /* 
    {
        "name": <ATTR NAME>,
        "type": <type>,
        "standby_wait": <STANDBY WAIT>,
        "next_showtimes": [<next showtime>, <following showtime>, etc.],
        "boarding_group": <min group> <max group>,
        "status": <STATUS STRING>
    }
    */
    // And transform it into
    /*
    {
        <attrNameKey + 0>: <attrName1>,
        <attrNameKey + 1>: <attrName2>,
        ... etc
    }
    */
    const payloadDict = {};
    var payload_idx = 0;
    clean_sorted_data.forEach(venue_data => {
        // Generate a status string to be put into the payload.
        // Priority for status string:
        // - wait time
        // - boarding group
        // - show time
        // - status
        var status_string = "";
        const standby_wait = venue_data["standby_wait"]
        const boarding_groups = venue_data["boarding_groups"];
        const showtimes = venue_data["next_showtimes"];
        if(standby_wait != null)
        {
            status_string = standby_wait + " minute";
            if(standby_wait != 1) status_string += "s";

        } else if (boarding_groups.length > 0)
        {
            status_string = "Groups " + boarding_groups[0] + "-" + boarding_groups[1];
        } else if (showtimes.length > 0)
        {
            const now = Date.now();
            const next_show = showtimes[0];
            const remaining_shows = showtimes.length - 1;
            // If there aren't any more shows after this one, and it's already started,
            // report its end time
            if(remaining_shows == 0 && (now > next_show[0]))
            {
                status_string = "Ends " + next_show[1].toLocaleTimeString('default', {timeStyle: 'short'});
            } else 
            {
                // Report the start time of the next show
                status_string = next_show[0].toLocaleTimeString('default', {timeStyle:'short'})
                if(remaining_shows > 0) status_string += " + " + remaining_shows + " more";
            }
        } else {
            status_string = venue_data["status"];
        }
        
        // Pack the attraction name into the payload.
        payloadDict[keys.i_attractionNameString + payload_idx] = venue_data["name"];
        // And the attraction status, too.
        payloadDict[keys.i_attractionStatus + payload_idx] = status_string;
        // Increment
        payload_idx += 1;
    });

    // Add in the number of attractions to the payload
    payloadDict[keys.i_attractionCount] = payload_idx;

    return payloadDict;
}

module.exports = AttractionsAPI;