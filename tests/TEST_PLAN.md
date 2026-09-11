# Test Plan

| Level | Area | Examples |
|---|---|---|
| Unit | APIManager | key precedence, URL construction, encoding, missing key |
| Unit | LocationManager | user context, storage initialization, history limits |
| Unit | UserDataManager | CSV round trip, malformed rows, favorites, searches, isolation, limits |
| Unit | WeatherStorageManager | save, clear, backup, active-user protection |
| Unit | WeatherDataProcessor | empty data, reports, historical scope |
| Unit | Weather | getter state, missing API key, failure state |
| Unit | Authentication | admin login, registration, hashing, role checks |
| Unit | Validation | username/password boundaries and allowed characters |
| Unit | Enhancement | thresholds, logs, export/insight failure handling |
| Integration | API + Location | configuration and location query URL boundary |
| Integration | API + Weather | URL construction and missing-key behavior |
| Integration | Location + Favorites | persistence, isolation, max-five rule |
| Integration | User Data + Storage | shared CSV and record coexistence |
| Integration | Weather Storage | location correctness, ordering, admin visibility |
| Integration | Reports | report generation and user scope |
| Integration | Authentication + Data | register/login persistence and hashing |
| Functional | Authentication | valid/invalid login and registration flows |
| Functional | Favorites | add, duplicate, fifth, sixth, remove |
| Functional | Search history | count, case-insensitive, ten/eleven boundary, user isolation |
| Functional | Weather/Forecast | missing API and controlled failure paths |
| Functional | Alerts/Insights | missing API, thresholds, empty configuration |
| Functional | Travel/Compare | incomplete input and data-isolation behavior |
| Functional | Export | no-session and existing-data behavior |
| Functional | Automatic Refresh | missing key, minimum interval, empty input |
| Functional | Admin Dashboard | API, thresholds, logs, stored data |
| System | Authentication | full registration/login and failure paths |
| System | User workflow | account + favorite + search + weather persistence |
| System | Admin workflow | configuration, thresholds, all-user data |
| System | Weather workflow | fetch failure, display, clear |
| System | Data management | backup, clear, preservation of non-weather records |
| System | Full application | launch/exit, invalid menu, admin logout, registration |
| System | Menu source checks | required user/admin options and logout |

## Important boundaries

- Favorites: 0, 1, 4, 5, 6 and duplicate cases.
- Recent searches: 0, 1, 10, 11 and per-user isolation.
- Username: empty, valid punctuation, 32 characters, 33 characters, invalid spaces.
- Password: six characters, below minimum, mismatch.
- Authentication: correct password, wrong password, wrong role, default admin.
- Storage: no active user, multiple users, weather/non-weather coexistence, backup and clear.
- API: configured key, missing key, encoded query, controlled failure.

## Non-deterministic/live testing

The current production implementation directly calls WeatherAPI/cURL and does not expose a dependency-injection interface for a fake HTTP client. Therefore the main automated suite avoids live network calls. A separate live smoke-test layer can be added later if the project is expected to validate real API responses against a test key.
