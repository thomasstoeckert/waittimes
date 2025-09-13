var default_data = require('./default-destinations-data');

function DestinationsAPI() {
    this.API_BASE_URL = "https://api.themeparks.wiki/v1/destinations";

    this.CACHE_KEY = "pwt_destinations_cache";
    this.CACHE_AGE_KEY = "pwt_destinations_cache_last_fetched";
    this.CACHE_MAX_AGE_DAYS = 7;

    this.cache_last_fetched = 0;
    this.cache_data = default_data;

    // Attempt to load data from the cache
    if(this.isDataInCache()) {
        const cache_data = this.getDataFromCache();
        this.cache_last_fetched = cache_data[0];
        this.cache_data = cache_data[1];
    } else {
        this.refreshCache( function () {} );
    }
}

DestinationsAPI.prototype.hitEndpoint = function(callback) {
    // Hit the endpoint, return the data
    const method = "GET";
    const url = this.API_BASE_URL;
    const request = new XMLHttpRequest();

    request.onreadystatechange = function() {
        if(this.readyState != 4) return;

        // Get the data / result
        if(this.status == 200) {
            // yay!
            try {
                const response = JSON.parse(this.responseText);
                const destinations = response.destinations;
                console.log("Got a destinations response with " + destinations.length + " entries");
                callback(response);
            } catch (e) {
                console.log("Error while hitting destinations endpoint!");
                console.log(e.name);
                console.log(e.message);
                console.log(e.stack);
            }

        } else {
            // Oh no :(
            console.log("Something went wrong with the response CODE :(");
            console.log(this.status);
            console.log(this.responseText);
        }
    }

    request.onerror = function() {
        // Something went wrong
        // Oh no :(
        console.log("Something went wrong with the request itself :(");
    }

    request.open(method, url);
    request.send();
}

DestinationsAPI.prototype.storeCache = function(data) {
    // Put the data into our localstorage
    this.cache_data = data;
    localStorage.setItem(this.CACHE_KEY, JSON.stringify(this.cache_data));
    // Update the cache_last_fetched guy
    this.cache_last_fetched = Date.now()
    localStorage.setItem(this.CACHE_AGE_KEY, this.cache_last_fetched);
}

DestinationsAPI.prototype.refreshCache = function(callback) {
    // Attempt to poll the endpoint for more data
    this.hitEndpoint((data) => {this.storeCache(data); callback()});
}

DestinationsAPI.prototype.isDataInCache = function() {
    const potential_data = this.getDataFromCache();
    if(potential_data[0] == null) return false;
    if(potential_data[1] == null) return false;
    return true;
}

DestinationsAPI.prototype.getDataFromCache = function() {
    return [
        Number.parseInt(localStorage.getItem(this.CACHE_AGE_KEY)),
        JSON.parse(localStorage.getItem(this.CACHE_KEY))
    ]
}

DestinationsAPI.prototype.getCacheAgeInDays = function() {
    if(this.cache_last_fetched == undefined) return Infinity;

    // Get today's timestamp
    var now = Date.now()
    var elapsed_milis   = now - this.cache_last_fetched;
    var elapsed_seconds = elapsed_milis   / 1000;
    var elapsed_minutes = elapsed_seconds / 60;
    var elapsed_hours   = elapsed_minutes / 60;
    var elapsed_days    = elapsed_hours   / 24;
    return elapsed_days;
}

DestinationsAPI.prototype.hasFreshDestinations = function() {
    return (this.cache_data != undefined) && (this.getCacheAgeInDays() < this.CACHE_MAX_AGE_DAYS);
}

DestinationsAPI.prototype.getDestinations = function() {
    return this.cache_data;
}

module.exports = DestinationsAPI;