# Define user token issuance and content ownership

Type: grilling
Status: resolved
Blocked by: 03, 24

## Question

How does the Cloud Media Service identify users, issue credentials, and record ownership of uploaded Shared Sounds without building a full account system for the competition demo?

## Answer

The Cloud Media Service retains a minimal User identity model because uploaded source audio and every generated content record must have an owner.

The backend issues a long-lived opaque User Token and returns its plaintext value only when it is created. SQLite stores the User identity and a one-way digest of the token rather than the recoverable plaintext token. A valid User Token identifies one `user_id` and authorizes that user to:

- upload a source audio file;
- create a new shareable content item owned by that user;
- query the processing state and metadata of their own uploads;
- list their own content;
- revoke or delete their own content when that operation is implemented.

The competition implementation does not require passwords, email verification, social login, refresh tokens, profiles, organizations, or account recovery. The User Token itself is the persistent client credential. The exact bootstrap UI for requesting the initial token belongs to the external Sharing Interface, while the backend owns token issuance and recognition.

Ownership controls upload and management operations, not NFC playback. Once a content item reaches `READY` and its Compact Content URL is written to an NFC card, the fixed authenticated Trigger and Playback devices may resolve and play it without becoming its owner. Multiple NFC cards may reference the same content, and the backend does not register physical card UIDs.
