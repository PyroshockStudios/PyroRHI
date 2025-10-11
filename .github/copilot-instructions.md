# Global Copilot Code Review Instructions

When reviewing pull requests:
- Verify that code adheres to the repository's C++ style: CamelCaseClass, mMember, bBoolean, kConstant, gGlobal, IInterface, enum struct instead of enum class, and C++ style casts.
- Ensure all new classes and public methods have documentation comments.
- Check for missing resource cleanup or leaks.
- Flag untested public APIs or unsafe pointer operations.
- Prefer const references and RAII patterns where possible.