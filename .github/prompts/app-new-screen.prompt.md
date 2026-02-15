# New App Screen

Create a new screen/page for the esp-fly-in-peace Flutter companion app.

## Input

- Screen name: {{SCREEN_NAME}} (e.g., `DeviceScanner`, `FlightDashboard`, `DeviceConfig`)
- Purpose: {{PURPOSE}}
- Navigation: {{HOW_TO_REACH}} (e.g., "Home → tap scan button", "Scanner → tap device")

## Output Structure

```
app/lib/
├── screens/
│   └── {{screen_name_snake}}/
│       ├── {{screen_name_snake}}_screen.dart    # Screen widget
│       └── {{screen_name_snake}}_state.dart     # State management (if needed)
├── widgets/
│   └── {{related_widget}}.dart                  # Reusable widgets for this screen
```

Test: `app/test/screens/{{screen_name_snake}}_test.dart`

## Requirements

1. **Architecture**: Follow the project's state management pattern (see Phase 0 setup).

2. **Responsive layout**:
   - Works in portrait mode on phones (360–430 dp wide)
   - Adapts to tablets (600+ dp wide) if applicable
   - Use `LayoutBuilder` or `MediaQuery` for responsive breakpoints

3. **State handling** — every screen must handle:
   - **Loading state**: Show a progress indicator
   - **Error state**: Show error message with retry action
   - **Empty state**: Show a helpful message (e.g., "No devices found")
   - **Connected/Disconnected**: If BLE-dependent, show connection status

4. **BLE awareness**:
   - If the screen depends on BLE data, handle disconnection gracefully
   - Show a "Disconnected" banner or redirect to scanner
   - Don't crash on null BLE data

5. **Navigation**:
   - Use the project's navigation approach (Navigator 2.0, GoRouter, or simple push)
   - Pass minimal data between screens (device ID, not full objects)

6. **Tests**:
   - Widget test for rendering in each state (loading, error, empty, data)
   - Unit test for state/business logic if present

7. **Comments**: English. Explain non-obvious widget choices or BLE interactions.

## Flutter Widget Guidelines

- Prefer `StatelessWidget` when possible; use `StatefulWidget` only for local UI state.
- Extract reusable widgets into `widgets/` directory.
- Use `const` constructors where possible.
- Handle `dispose()` properly — cancel streams, timers, subscriptions.
- Use `Theme.of(context)` for colors/text styles, not hardcoded values.
