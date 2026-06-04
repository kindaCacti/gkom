# gkom

## Running Relase

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Potential memory leaks

Oftentimes when trying to use `-fsanitize=address` it detects leaks that we are cannot control. I got `SUMMARY: AddressSanitizer: 263727 byte(s) leaked in 6130 allocation(s).` when trying to compile this project at some point. What happens is that gpu drivers dealocate memory when the process ends, and that is after the sanitizer stops working. This makes the sanitizer as well as some other tools like valgrind think that there are leaks even though there are no actual leaks. Unfortunately there are no real solutions for this problem. We can either suppress all the data from the leaks, or try to live with those that happen, and try to only look into the leaks that increase the number of bytes leaked.

## Benchmarks

##### Maximum number of bullets to sustain a certain fps level

| Device | CPU           | GPU      | OS      | Instancing | 30 fps | 60 fps |
| ------ | ------------- | -------- | ------- | ---------- | ------ | ------ |
| Laptop | i7-9750H      | UHD 630  | Windows | On         | ~75k   | ~30k   |
| Laptop | i7-9750H      | UHD 630  | Windows | Off        | ~16k   | ~8k    |
| PC     | Ryzen 9 9950X | RTX 5070 | Linux   | On         | ~300k  | ~200k  |
| PC     | Ryzen 9 9950X | RTX 5070 | Linux   | Off        | ~40k   | ~20k   |
