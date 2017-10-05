# WebUI

### Description
Contains multi-user web interface, allowing control watering and see logs.
Each user has own PLC controller and own html template of UI. Server communicates with PLC controolers via HTTP API, reads status and send commands.
Written on PHP. Configuration and logs are stored in SQLite database.
