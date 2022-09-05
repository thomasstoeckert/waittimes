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
                "defaultValue": "Visibility"
            },
            {
                "type": "toggle",
                "messageKey": "c_showEmpty",
                "label": "Show Empty Waits",
                "defaultValue": false,
                "description": "If true, the watchapp will show attractions with no posted wait or status. If false, it will hide these."
            },
            {
                "type": "toggle",
                "messageKey": "c_showValidDataOnly",
                "label": "Show Valid Data Only",
                "defaultValue": false,
                "description": "If true, the watchapp will show entries only when there exists valid data (wait time, showtime, or boarding group)"
            },
            {
                "type": "toggle",
                "messageKey": "c_showShowtimes",
                "label": "Show Showtimes",
                "defaultValue": true,
                "description": "If true, show in-park shows and their next upcoming showtime, if available"
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