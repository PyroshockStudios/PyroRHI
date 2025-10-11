# PyroRHI Core Review Focus

When reviewing PyroRHI core code:
- Focus on interface clarity and API consistency across backends.
- Ensure no backend-specific logic leaks into core abstractions.
- Verify that event or resource classes maintain cross-platform correctness.
- Check that enums and naming conventions follow PascalCase (RHI standard).
- Check that relevant structures and functions have doxygen-style comments, if it's not trivially clear.
