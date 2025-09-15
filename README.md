# OpenVINS example project

### Build Docker

```bash
docker build -t develop .
```

### Enter Docker

```bash
docker run \
    -it \
    -v "$PWD":/workspace \
    -w /workspace \
    develop
```
