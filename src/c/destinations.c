#include "./destinations.h"
#include "destinations.h"

static char s_park_names[I_MAX_PARKS][I_MAX_PARK_NAME_LENGTH];
static char s_park_ids[I_MAX_PARKS][I_PARK_UUID_LENGTH];
static char s_destination_names[I_MAX_DESTINATIONS][I_MAX_DESTINATION_NAME_LENGTH];
static int i_park_destination[I_MAX_PARKS];
static int i_park_count;
static int i_destination_count;

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

    // Collect the number of destinations we're about to recevei
    Tuple *destcount_tuple = dict_find(iter, MESSAGE_KEY_c_newpark_destcount);
    if(!destcount_tuple) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "[D.C]: Failed to find the destcount tuple. Returning.");
        return 0;
    }
    
    i_destination_count = destcount_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: Received word that we'll be seeing names for %d destinations", i_destination_count);
    if(i_destination_count <= 0) {
        APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: That's <= Destinations. Proceeding no further.");
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
        strncpy(s_park_ids[newpark_index], parkid_tuple->value->cstring, I_PARK_UUID_LENGTH - 1);
        i_park_destination[newpark_index] = destid_tuple->value->int32;

        // Log that we've parsed this bad boy.
        APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: {%2d}(%s)[%2d]%s", 
            newpark_index, s_park_ids[newpark_index], i_park_destination[newpark_index], s_park_names[newpark_index]);
    }

    // Now, we need to collect our destination information.
    for (int destination_index = 0; destination_index < i_destination_count; destination_index++) {
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

void persist_save_destinations_data()
{
    // Prepare metadata block
    PersistDestMetadata persist_dest_metadata = (PersistDestMetadata) 
    {
        .num_parks = i_park_count,
        .num_destinations = i_destination_count,
        .version = PERSIST_VERSION_DESTINATIONS
    };

    // Store metadata block
    persist_write_data(PERSIST_KEY_DESTINATIONS_METADATA, &persist_dest_metadata, sizeof(PersistDestMetadata));

    // Store park data
    PersistParkData working_park_persist[4];
    int source_park_index = 0;
    while (source_park_index < i_park_count)
    {
        int working_index = source_park_index % 4;
        
        // For each park, put it into the working_park_persist array
        working_park_persist[working_index].park_destid_persist = i_park_destination[source_park_index];

        strncpy(working_park_persist[working_index].park_name_persist,
            s_park_names[source_park_index], I_MAX_PARK_NAME_LENGTH - 1);

        strncpy(working_park_persist[working_index].park_uuid_persist,
            s_park_ids[source_park_index], I_PARK_UUID_LENGTH - 1);
        
        if(source_park_index % 4 == 3)
        {
            // Save
            int persist_key_offset = source_park_index / 4;
            persist_write_data(PERSIST_KEY_PARKS_BASE + persist_key_offset, 
                working_park_persist, sizeof(PersistParkData) * 4);
        }

        source_park_index += 1;
    }

    // If the park count didn't divide evenly into sets of four parks, the last
    // set of (1-3) won't have been stored. Store that.
    if(i_park_count % 4 != 0) 
    {
        int persist_key_offset = i_park_count / 4;
        persist_write_data(PERSIST_KEY_PARKS_BASE + persist_key_offset,
            working_park_persist, sizeof(PersistParkData) * 4);
    }

    // Store destinations data
    //
    // We can store six destination names in a 256-byte block 
    // (37 bytes per name) = 222 bytes
    char working_destnames[6][I_PARK_UUID_LENGTH];
    // Store'm
    for (int i = 0; i < i_destination_count; i++) 
    {
        int working_destnames_index = i % 6;
        
        strncpy(working_destnames[working_destnames_index], 
            s_destination_names[i], I_MAX_DESTINATION_NAME_LENGTH - 1);
            
        if(working_destnames_index == 5)
        {
            int persist_key_offset = i / 6;
            persist_write_data(PERSIST_KEY_DESTINATIONS_BASE + persist_key_offset, 
                working_destnames, sizeof(char) * I_PARK_UUID_LENGTH * 6);
        }
    }

    APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: Stored %d Parks from %d Destinations into storage", i_park_count, i_destination_count);
}

void persist_load_destinations_data()
{
    // Get metadata block
    PersistDestMetadata persist_dest_metadata;
    if(!persist_exists(PERSIST_KEY_DESTINATIONS_METADATA))
    {
        // If it doesn't exist, we can't continue. Print an error, and fail
        APP_LOG(APP_LOG_LEVEL_ERROR, "[D.C]: Unable to find any destinations data in storage. Not continuing load.");
        return;
    }
    // APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: Found metadatablock. Loading persist data from storage");
    
    persist_read_data(PERSIST_KEY_DESTINATIONS_METADATA, &persist_dest_metadata, sizeof(PersistDestMetadata));
    
    // Print out that we've got it
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "[D.C]: Got the metadata block. Looking for %d parks and %d destination names, v%d",
    //     persist_dest_metadata.num_parks, persist_dest_metadata.num_destinations,
    //     persist_dest_metadata.version);

    if(persist_dest_metadata.version != PERSIST_VERSION_DESTINATIONS)
    {
        APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: Version loaded from metadata block doesn't match expected version. %d != %d.", persist_dest_metadata.version, PERSIST_VERSION_DESTINATIONS);
        APP_LOG(APP_LOG_LEVEL_WARNING, "[D.C]: No 'upgrade' functionality implemented. Returning.");
        return;
    }
    
    i_park_count = persist_dest_metadata.num_parks;
    i_destination_count = persist_dest_metadata.num_destinations;

    // Load parks data
    // Parks data is stored in 4-park data blocks. Estimate the number of data
    // blocks we should read
    int expected_park_blocks = i_park_count / 4 + (i_park_count % 4 != 0);
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "I think we'll be seeing %d blocks of 4 parks, since we have %d total parks", expected_park_blocks, i_park_count);
    int park_idx = 0;
    for (int i = 0; i < expected_park_blocks; i++)
    {
        // Load a park block
        PersistParkData working_park_persist[4];
        persist_read_data(PERSIST_KEY_PARKS_BASE + i, working_park_persist, 
            sizeof(PersistParkData) * 4);

        
        // Pull out the amount of data I expect to be in it
        int remaining_count = i_park_count - park_idx;
        // Clamp between 0 and 4
        int set_count = remaining_count;
        if(set_count > 4)
        {
            set_count = 4;
        }

        // APP_LOG(APP_LOG_LEVEL_DEBUG, "Read a block from %d! I think we'll pull %d parks from it", PERSIST_KEY_PARKS_BASE + i, set_count);
        for (int j = 0; j < set_count; j++)
        {
            // Store that in big original array
            i_park_destination[park_idx] = working_park_persist[j].park_destid_persist;
            strncpy(s_park_names[park_idx], working_park_persist[j].park_name_persist, I_MAX_PARK_NAME_LENGTH - 1);
            strncpy(s_park_ids[park_idx], working_park_persist[j].park_uuid_persist, I_PARK_UUID_LENGTH - 1);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "Read a park! Here's its data: ");
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "  Index: %d", park_idx);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "  DestIDX: %d", i_park_destination[park_idx]);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "  Name: %s", s_park_names[park_idx]);
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "  Id: %s", s_park_ids[park_idx]);
            park_idx += 1;
        }
    }

    // Load destinations data
    // Dest data is stored in 6-name blocks. Estimate the number of
    // data blocks that should be read
    int expected_dest_blocks = i_destination_count / 6 + (i_destination_count % 6 != 0);
    int dest_idx = 0;
    for (int i = 0; i < expected_dest_blocks; i++)
    {
        // Load a dest block
        char working_destnames[6][I_PARK_UUID_LENGTH];
        persist_read_data(PERSIST_KEY_DESTINATIONS_BASE + i,
            working_destnames, sizeof(char) * I_PARK_UUID_LENGTH * 6);

        // Pull our the number of names we expect to be in it
        int remaining_count = i_destination_count - dest_idx;
        int set_count = remaining_count;
        if(set_count > 6)
        {
            set_count = 6;
        }

        // Store that in the big array
        for (int j = 0; j < set_count; j++)
        {
            strncpy(s_destination_names[dest_idx], working_destnames[j], 
                I_MAX_DESTINATION_NAME_LENGTH - 1);
            dest_idx += 1;
        }
    }

    APP_LOG(APP_LOG_LEVEL_INFO, "[D.C]: Loaded %d Parks from %d Destinations from storage", i_park_count, i_destination_count);
}
