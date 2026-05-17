var is_time_broken = false;

function safe_localeCompare(string_a, string_b) {
    // First, try the localecompare
    if(!is_time_broken) {
        // Try this
        try {
            return string_a.localeCompare(string_b);
        } catch (error) {
            console.log("Something went wrong with localecompare. Falling back on non-locale stuff");
            is_time_broken = true;
        }
    }
    // Fallback, if the localecompare broken
    const a_lower = string_a.toLowerCase();
    const b_lower = string_b.toLowerCase();
    if(a_lower < b_lower) return -1;
    if(a_lower > b_lower) return 1;
    return 0;
}

function safe_timestring(datetime) {
    // First try the timestring
    if(!is_time_broken) {
        try {
            return datetime.toLocaleTimeString('default', {timeStyle:'short'});
        } catch (error) {
            console.log("Something went wrong with timestring. Falling back on defaults.");
            is_time_broken = true;
        }
    }

    // fallback
    let hours = datetime.getHours();
    let minutes = datetime.getMinutes();
    const ampm = hours > 12 ? 'PM' : 'AM';
    hours = hours % 12
    hours = hours ? hours : 12;
    minutes = minutes < 10 ? '0' + minutes : minutes;
    const strTime = hours + ":" + minutes + ' ' + ampm;
    return strTime
}

module.exports = {safe_localeCompare, safe_timestring};