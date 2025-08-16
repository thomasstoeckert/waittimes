module.exports = [
    {
        "type": "heading",
        "defaultValue": "WaitTimes Configuration"
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Venue Display Settings"
            },
            {
                "type": "checkboxgroup",
                "messageKey": "c_typeVisiblity",
                "label": "Visible Venue Types",
                "defaultValue": [true, true],
                "options": ["Attractions", "Shows"]
            },
            {
                "type": "toggle",
                "messageKey": "c_hideClosed",
                "label": "Hide Closed Venues",
                "defaultValue": true,
                "description": "If checked, venues who are completely closed will be hidden. Venues who are reporting a 'down' or 'delayed' status will continue to be displayed."
            },
            {
                "type": "radiogroup",
                "messageKey": "c_sortOrder",
                "label": "Venue Sort Order",
                "defaultValue": "wait_decrease",
                "options": [
                    {
                        "label": "A->Z",
                        "value": "alpha_increase"
                    },
                    {
                        "label": "Z->A",
                        "value": "alpha_decrease"
                    },
                    {
                        "label": "Shortest Waits First",
                        "value": "wait_increase",
                    },
                    {
                        "label": "Longest Waits First",
                        "value": "wait_decrease"
                    },
                    {
                        "label": "Earliest Showtimes First",
                        "value": "shows_increase"
                    },
                    {
                        "label": "Latest Showtimes First",
                        "value": "shows_decrease"
                    }
                ]
            }
        ]
    },
    {
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
                "type": "checkboxgroup",
                "messageKey": "c_showParks",
                "label": "Visible Parks",
                "defaultValue": [
                    true, true, false, false, false, true, true, true, true, true, false, false, true, true, false, true, false, false,
                    true, true, false, true, true, true, false, true, true, true, false, false, true, false, false, true, false, true,
                    true, false, true, true, false, true, false, false, true, true, false, false, false, false, false, true, false,
                    false, false, false, true, true, true
                ],
                "options": [
                    "Disneyland Park",
                    "Disney's California Adventure",
                    "California's Great America",
                    "Knott's Berry Farm",
                    "SeaWorld San Diego",
                    "Universal Studios (Hollywood)",
                    "Magic Kingdom",
                    "Epcot",
                    "Hollywood Studios",
                    "Animal Kingdom",
                    "Typhoon Lagoon",
                    "Blizzard Beach",
                    "Universal Studios (Florida)",
                    "Universal Islands of Adventure",
                    "Volcano Bay",
                    "SeaWorld Orlando",
                    "Aquatica Orlando",
                    "Legoland Florida",
                    "Busch Gardens Tampa Bay",
                    "Busch Gardens Williamsburg",
                    "Canada's Wonderland",
                    "Carowinds",
                    "Cedar Point",
                    "Dollywood",
                    "Dorney Park",
                    "Hersheypark",
                    "Kings Dominion",
                    "Kings Island",
                    "Michigan's Adventure",
                    "SeaWorld San Antonio",
                    "Silver Dollar City",
                    "Valleyfair",
                    "Worlds of Fun",
                    "Alton Towers",
                    "Chessington World of Adventures",
                    "Legoland Windsor",
                    "Thorpe Park",
                    "Europa Park",
                    "Rulantica",
                    "Phantasialand",
                    "Heide Park",
                    "Walibi Holland",
                    "Disneyland Paris",
                    "Walt Disney Studios Park",
                    "Parc Asterix",
                    "Bellewaerde",
                    "Plopsaland De Panne",
                    "Holiday Park",
                    "Liseberg",
                    "Efteling",
                    "Attractiepark Toverland",
                    "Gardaland",
                    "PortAventura Park",
                    "Ferrari Land",
                    "Aquatic Park",
                    "Hong Kong Disneyland",
                    "Shanghai Disneyland",
                    "Tokyo Disneyland",
                    "Tokyo DisneySea"
                ]
            }
        ]
    },
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    }
]