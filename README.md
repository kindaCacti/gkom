# gkom

## Running Relase

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Potential memory leaks

Oftentimes when trying to use `-fsanitize=address` it detects leaks that we are cannot control. I got `SUMMARY: AddressSanitizer: 263727 byte(s) leaked in 6130 allocation(s).` when trying to compile this project at some point. What happens is that gpu drivers dealocate memory when the process ends, and that is after the sanitizer stops working. This makes the sanitizer as well as some other tools like valgrind think that there are leaks even though there are no actual leaks. Unfortunately there are no real solutions for this problem. We can either suppress all the data from the leaks, or try to live with those that happen, and try to only look into the leaks that increase the number of bytes leaked.

## Benchmarks

##### Benchmarked devices:

| Device | CPU           | GPU      | OS      |
| ------ | ------------- | -------- | ------- |
| Laptop | i7-9750H      | UHD 630  | Windows |
| PC     | Ryzen 9 9950X | RTX 5070 | Linux   |

##### Maximum number of bullets to sustain a certain fps level:

| Device | Collisions | Instancing | 30 fps | 60 fps |
| ------ | ---------- | ---------- | ------ | ------ |
| PC     | Off        | Off        | ~40k   | ~20k   |
| PC     | Off        | On         | ~400k  | ~220k  |
| PC     | On         | Off        | ~40k   | ~20k   |
| PC     | On         | On         | ~280k  | ~170k  |
| Laptop | Off        | Off        | -      | -      |
| Laptop | Off        | On         | -      | -      |
| Laptop | On\*       | Off        | ~16k   | ~8k    |
| Laptop | On\*       | On         | ~75k   | ~30k   |
