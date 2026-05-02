## Campus Announcements – Project Manager
**Project ID: ** RA2311003020057

### Project Summary
Campus Notifications is a React based frontend application to manage the university campus notifications (Events, Results, Placements). The app calls the backend service API to perform the evaluation.
--- ## Tech Stack 
- **Frontend:** React 19 + TypeScript + Vite 6 

- **Styling:** Tailwind CSS 4 

- **Tool:** Vite

- **Backend Integration:** Express 4 

- **Icons:** Lucide React 

- **Routing:** React Router v7 

- **Animation**: Movement

- ** AI Integration:** Google GenAI Sdk

--- 

## Quick Start Commands ### Install Dependencies ```bash npm install ```

### Development server ```bash npm run dev # Starts on http://localhost:3000 ```

### Build Production ```bash npm run build ```

### Build Preview ```bash npm run preview ```

### Code quality ```bash npm run lint # TypeScript type checking ```

### Clean up ```bash npm run clean # Removes dist/ folder ```

# Project Architecture ## Development Setup- **First Try:** Next.js + MUI/Emotion styling (discarded due to compatibility issues)
- **Current Approach:** Clean React + TypeScript with hardcoded components and Tailwind CSS for optimized lightweight implementation- **Assets:** Added Feather Icons icon library to /Assets folder

- **Middleware:** Custom logging middleware located in `/logging_middleware`

### Directory Structure ``` src/ ├── components/ # React UI components 
  ├── hooks/ # Custom React hooks 
  ├── pages/ # Page components 
  ├── lib/ # Utilities and services
  ├── types/ # Typescript definitions
  └── styles/ # CSS files logging_middleware/ # Logging middleware Assets/ # Icons and other static assets

--- ## API Integration ### Base URL ``` http://20.207.122.201/evaluation-service ```

### Notification Endpoints 

#### 1. Retrieve All Notifications (Default) ```bash curl -X GET "http://20.207.122.201/evaluation-service/notifications" \
  -H 'Content-Type: application/json'

#### 2. Get with Pagination ```bash curl -X GET "http://20.207.122.201/evaluation-service/notifications?page=1&limit=10" \-H "Content-Type: application/json"

#### 3. Choose a Notification Type
supported types: Event, Result, Placement
**Get Event Notifications:** ```bash curl -X GET "http://20.207.122.201/evaluation-service/notifications?notification_type=Event" \
-H "Content-Type: application/json"


**Get Results Notifications:** ```bash curl -X GET "http://20.207.122.201/evaluation-service/notifications?notification_type=Result" \ -H "Content-Type: application/json"

**Get Placement Notifications** ```bash curl -X GET "http://20.207.122.201/evaluation-service/notifications?notification_type=Placement" \ -H "Content-Type: application/json"

#### 4. Combined Query (Type + Pagination) ```bash curl -X GET "http://20.207.122.201/evaluation-service/notifications?notification_type=Placement&page=2&limit=5" \ -H "Content-Type: application/json"

#### 5. Debug Mode (Verbose) ```bash curl -v -X GET "http://20.207.122.201/evaluation-service/notifications?notification_type=Event&limit=5"

--- 

## Middleware ### Logging Middleware Library
In /logging_middleware:- Custom request/response log
- Support for TypeScript
- Modular package structure
- See `/logging_middleware/README.md` for more details

### Implementation 
- middleware is added to log:- API request and response
- Error Management
- Time of the request

- Data processing events 

--- 

## Key Implementation Points

✅ **Done:**- Vite React + Typescript setup
- Component structure and routing 

- API integration infrastructure- Implementing Logging Middleware

- assets/icons integration

⚙️ **On Going:**- UI for filtering and paginating notifications
- Elements of the notification card
- Notification management hooks
- Error handling and corner cases

--- 

### Development guidelines
// Write new code in TypeScript
- Follow component structure in `/src/components` 
- Add types in `/src/types/index.ts`

- Use hooks from `/src/hooks`

- api calls via `/src/lib/api.ts`

--- 

## Troubleshooting | Issue | Solution | |-------|----------| | Port 3000 in use already | Change port in `npm run dev` or kill the process | | Module not found errors | Run `npm install` and make sure all dependencies are installed |
| TypeScript errors | Run `npm run lint` for issues | | Vite build issues | Run `npm run clean` and rebuild |