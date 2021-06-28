module.exports = [
    {
        "type": "heading",
        "defaultValue": "WaitTimes Park Configuration"
    },
    {
        "type": "text",
        "defaultValue": "Select the parks you'd like to see on your watch."
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Parks"
            },
            {
                "type": "checkboxgroup",
                "messageKey": "c_showPark",
                "label": "Visible Parks",
                "defaultValue": [true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true],
                "options": [
                    "Magic Kingdom",
                    "EPCOT",
                    "Hollywood Studios",
                    "Animal Kingdom",
                    "Universal Florida",
                    "Islands of Adventure",
                    "Volcano Bay",
                    "Disneyland",
                    "Disney's California Adventure",
                    "Tokyo Disneyland",
                    "Tokyo DisneySea",
                    "Disneyland Paris",
                    "Walt Disney Studios Paris",
                    "Hong Kong Disneyland",
                    "Shanghai Disneyland",
                    "Universal Hollywood",
                    "Europa Park",
                    "Efteling"
                ]
            }
        ]
    },
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    }
]