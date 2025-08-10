#include "attractions_data.h"

static char s_attraction_names[I_MAX_ATTRACTIONS][I_MAX_ATTRACTION_NAME_LENGTH];
static char s_attraction_status[I_MAX_ATTRACTIONS][I_MAX_ATTRACTION_STATUS_LENGTH];
static int  s_num_attractions = 0;

int parse_attractions_response(DictionaryIterator *iter, void *context)
{
    APP_LOG(APP_LOG_LEVEL_INFO, "[A_D.C]: Attractions Parsing Requested. Hunting for data.");

    Tuple *attractioncount_tuple = dict_find(iter, MESSAGE_KEY_i_attractionCount);
    if(!attractioncount_tuple)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "[A_D.C]: Failed to find the attractioncount tuple. Returning.");
        return -1;
    }

    s_num_attractions = attractioncount_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "[A_D.C]: Received word that we'll be seeing data for %d attractions.", s_num_attractions);
    
    // Clear existing data
    memset(s_attraction_names, 0, sizeof(char) * I_MAX_ATTRACTIONS * I_MAX_ATTRACTION_NAME_LENGTH);
    memset(s_attraction_status, 0, sizeof(char) * I_MAX_ATTRACTIONS * I_MAX_ATTRACTION_STATUS_LENGTH);
    
    if(s_num_attractions <= 0) {
        APP_LOG(APP_LOG_LEVEL_INFO, "[A_D.C]: That's <= 0 attractions. Proceeding no further.");
        return 0;
    }

    // Collect our data
    for(int i = 0; i < s_num_attractions; i++)
    {
        Tuple *name_tuple = dict_find(iter, MESSAGE_KEY_i_attractionNameString + i);
        Tuple *status_tuple = dict_find(iter, MESSAGE_KEY_i_attractionStatus   + i);

        if(!(name_tuple && status_tuple))
        {
            APP_LOG(APP_LOG_LEVEL_ERROR, "[A_D.C]: Unable to find all data for attraction at index %d", i);
            return -1;
        }

        // Copy data into source string
        strncpy(s_attraction_names[i],  name_tuple->value->cstring, I_MAX_ATTRACTION_NAME_LENGTH - 1);
        strncpy(s_attraction_status[i], status_tuple->value->cstring, I_MAX_ATTRACTION_STATUS_LENGTH - 1);

        APP_LOG(APP_LOG_LEVEL_INFO, "[A_D.C]: {%3d}(%s)%s", i, 
            s_attraction_names[i],
            s_attraction_status[i]);
    }

    return 0;
}

void print_attractions()
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "[A_D.C]: Current Attractions Data --------");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "[A_D.C]: Attraction Count: %d", s_num_attractions);
    if(s_num_attractions > 0)
    {
        for(int i = 0; i < s_num_attractions; i++)
        {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "[A_D.C]>> [%3d]%s", i, s_attraction_names[i]);
            APP_LOG(APP_LOG_LEVEL_DEBUG, "[A_D.C]>>> %s", s_attraction_status[i]);
        }
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "[A_D.C]: No More Attractions Data --------");
}

int get_attractions_count()
{
    return s_num_attractions;
}

int get_attraction_at_index(int index, AttractionData *attraction_data)
{
    if(index < 0 || index >= s_num_attractions)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "[A_D.C]: Was asked for attraction data for an invalid index (0 <= %d < %d failed)", index, s_num_attractions);
        return -1;
    }

    attraction_data->attraction_name = s_attraction_names[index];
    attraction_data->attraction_status = s_attraction_status[index];
    return 0;
}
