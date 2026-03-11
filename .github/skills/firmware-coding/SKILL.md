---
name: firmware-coding
description: Use this skill when writing new features, fixing bugs, or refactoring code. 
---

# Firmware Development Workflow

This skill ensures all code development follows best practices for firmware development, including comprehensive test coverage.

## When to Activate

- Writing new features or functionality
- Fixing bugs or issues
- Refactoring existing code
- Adding API endpoints
- Creating new components

## File Organization

```
micro/
├── components/
│   ├── <component_name>/
│   │   ├── inc
│   │   |   ├── <component_name>.h
│   │   ├── src
│   │   |   ├── <component_name>.c
│   │   |   ├── <component_name>_hardware.c
│   │   |   ├── <component_name>_model.c
│   │   ├── CMakeLlists.txt
```

## Code Style

- Format code with `pe-code-tool format` after editing files
```bash
# Runs before every commit
pe-code-tool format <file_name>
```
- Act as a **pragmatic Clean Code / Clean Architecture mentor**.
- Be direct. Prioritize clarity, simplicity, and maintainability.
- Use `esp_err_t` for operations. Use `bool` for task runners.
- Use `ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE` for logging. Never `printf`.
- Validate all pointer parameters at public function entry.
- Prefer static allocation. Use `static` for file-scope functions.
- Use `snake_case` for functions/variables, `UPPER_SNAKE_CASE` for macros, `_t` suffix for types.
- Include guard `#ifndef`/`#define`, `extern "C"` wrapper, Doxygen for public API in headers.
- No file headers or top-of-file comment banners in `.c` files.
- No comments in `.c` files. Code must be self-explanatory through clear naming. No inline comments, no section separators, no `@brief` inside implementation files.
- Do not repeat yourself. Extract shared logic into well-named helper functions.
- Function and variable names must convey intent. If a comment is needed, rename the symbol instead.
- Use
```c
    if(a)
      return;
```
instead of
```c
    if(a)
    {
      return;
    }
```
- Use `!` for negative conditions instead of `== false` or `== NULL`.
- Use
```c
    bool ret = func_a();
    ret? ret = func_b(): ret; 
    ret? ret = func_c(): ret; 
    ret? ret = func_d(): ret;
    return ret;
```
or
```c
    esp_err_t ret = func_a();
    ret? ret = func_b(): ret; 
    ret? ret = func_c(): ret; 
    ret? ret = func_d(): ret;
    return ret;
```
when possible

## Best Practices

3. **Descriptive Test Names** - Explain what is


