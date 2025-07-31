#include "./destinations.h"
#include "destinations.h"

static char s_park_names[I_MAX_PARKS][I_MAX_PARK_NAME_LENGTH];
static char s_park_ids[I_MAX_PARKS][I_PARK_UUID_LENGTH];
static char s_destination_names[I_MAX_DESTINATIONS][I_MAX_DESTINATION_NAME_LENGTH];
static int i_park_destination[I_MAX_PARKS];
static int i_park_count;

int parse_destinations_response(DictionaryIterator *iter, void *context){
    // We've been called. We need to look for a message with our new park count
    // in it. If we see that, we can proceed.

    APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: Destinations Parsing Requested. Hunting for data.");

    Tuple *parkcount_tuple = dict_find(iter, MESSAGE_KEY_c_newpark_count);
    if(!parkcount_tuple) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "[D.C]: Failed to find the parkcount tuple. Returning.");
        return DESTINATIONS_ERROR;
    }

    // Collect the number of parks we're to receive
    i_park_count = parkcount_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: Received word that we'll be seeing data for %d parks", i_park_count);
    if(i_park_count <= 0) {
        APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: That's <= 0 parks. Proceeding no further.");
        return 0;
    }

    // Start collecting information for each park. Start by clearing the
    // existing data arrays.
    //
    // Probably unecessary. But! Doing it anyway.
    memset(s_park_names, 0, sizeof(char) * I_MAX_PARKS * I_MAX_PARK_NAME_LENGTH);
    memset(s_park_ids, 0, sizeof(char) * I_MAX_PARKS * I_PARK_UUID_LENGTH);
    memset(s_destination_names, 0, sizeof(char) * I_MAX_DESTINATIONS * I_MAX_DESTINATION_NAME_LENGTH);
    memset(i_park_destination, 0, sizeof(u_char) * I_MAX_PARKS);

    // Parse the data for our incoming parks
    int i_max_destid = -1;
    for (int newpark_index = 0; newpark_index < i_park_count; newpark_index++)
    {
        Tuple *parkname_tuple = dict_find(iter, MESSAGE_KEY_c_newpark_names   + newpark_index);
        Tuple *parkid_tuple   = dict_find(iter, MESSAGE_KEY_c_newpark_ids     + newpark_index);
        Tuple *destid_tuple   = dict_find(iter, MESSAGE_KEY_c_newpark_destids + newpark_index);
        
        // Check to see if we were able to fetch all information for this destination
        if(!(parkname_tuple && parkid_tuple && destid_tuple)) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "[D.C]: Unable to find all park info for park with index %d in the config array", newpark_index);
            return DESTINATIONS_ERROR;
        }

        // We're good! Go through and extract our data for this park
        strncpy(s_park_names[newpark_index], parkname_tuple->value->cstring, I_MAX_PARK_NAME_LENGTH - 1);
        strncpy(s_park_ids[newpark_index], parkid_tuple->value->cstring, I_PARK_UUID_LENGTH );
        i_park_destination[newpark_index] = destid_tuple->value->int32;

        // Next, see if we don't have this destination index yet.
        if(i_park_destination[newpark_index] > i_max_destid) {
            // We don't have it! Mark down that we have at *least* the number of dest IDs
            // for this park to exist.
            i_max_destid = i_park_destination[newpark_index];
        }

        // Log that we've parsed this bad boy.
        APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: {%2d}(%s)[%2d]%s", 
            newpark_index, s_park_ids[newpark_index], i_park_destination[newpark_index], s_park_names[newpark_index]);
    }

    // Now, we need to collect our destination information.
    for (int destination_index = 0; destination_index <= i_max_destid; destination_index++) {
        // This is easy - just the destination name
        Tuple *destname_tuple = dict_find(iter, MESSAGE_KEY_c_newpark_destnames + destination_index);
        if(!destname_tuple) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "[D.C]: Unable to find a destination name for index %d in the config array", destination_index);
            return DESTINATIONS_ERROR;
        }

        // We got it!
        strncpy(s_destination_names[destination_index], destname_tuple->value->cstring, I_MAX_DESTINATION_NAME_LENGTH - 1);
        APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: D{%2d}%s", destination_index, s_destination_names[destination_index]);
    }

    // We've got it all!
    APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: Finished parsing all destination information");
    return i_park_count;
}

void print_destinations()
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "[D.C]: Current Destinations Data -------");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "[D.C]: Park Count: %d", i_park_count);
    if(i_park_count > 0) {
        // Print info for each park
        for (int i = 0; i < i_park_count; i++) {
            int park_dest_id = i_park_destination[i];
            APP_LOG(APP_LOG_LEVEL_DEBUG, "[D.C]>> {%2d}[DESTID %2d](PARKID: %s)",
                i, park_dest_id, s_park_ids[i]);
            APP_LOG(APP_LOG_LEVEL_DEBUG, "[D.C]>>> (%s)%s",
                s_destination_names[park_dest_id], s_park_names[i]);
        }
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "[D.C]: No More Destinations Data -------");
}

int get_park_count()
{
    return i_park_count;
}

int get_data_for_park_index(int park_index, ParkData *park_data)
{
    if(park_index < 0 || park_index >= i_park_count)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "[D.C]: Was asked for park data for a park with invalid index %d [0, %d)", park_index, i_park_count);
        return -1;
    }

    park_data->park_name = s_park_names[park_index];
    park_data->park_uuid = s_park_ids[park_index];
    park_data->destination_name = s_destination_names[i_park_destination[park_index]];
    return 0;
}
