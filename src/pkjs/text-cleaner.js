module.exports = function (in_text, max_length=32) {
    // Given any normal unicode text, convert it into whatever fits
    // inside the Pebble's typical character set (and trim it to fit)
    var output = "";
    var string_length = in_text.length < max_length ? in_text.length : max_length;
    for (var i = 0; i < string_length; i++) {
        if(in_text.charCodeAt(i) <= 127) {
            output += in_text[i];
        }
    }
    return output;
}