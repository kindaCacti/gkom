# gkom

## Potential memory leaks

Oftentimes when trying to use `-fsanitize=address` it detects leaks that we are cannot control. I got `SUMMARY: AddressSanitizer: 238341 byte(s) leaked in 5086 allocation(s).` when trying to compile this project at some point. What happens is that gpu drivers dealocate memory when the process ends, and that is after the sanitizer stops working. This makes the sanitizer as well as some other tools like valgrind think that there are leaks even though there are no actual leaks. Unfortunately there are no real solutions for this problem. We can either suppress all the data from the leaks, or try to live with those that happen.