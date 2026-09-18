# ImgSorting

재미로 만든 이미지 정렬 시각화. 이미지를 세로 100조각으로 섞은 뒤 정렬 알고리즘 38종으로 복원함.

## 실행

[Releases](https://github.com/FMsongX2/ImgSorting/releases)에서 받아 실행.

- Windows: `ImgSorting-windows-x64.zip`
- macOS: `ImgSorting-macos-universal.zip`. 실행이 막히면 `xattr -d com.apple.quarantine ImgSorting`
- Linux: `ImgSorting-linux-x64.tar.gz`

Controls 창에서 이미지·WAV 불러오기, 시작·정지, 정렬 선택.

## Podman (Linux)

```bash
podman build -t imgsorting .
podman run --rm --userns=keep-id --security-opt label=disable -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix imgsorting
```

## 빌드

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DIMGSORTING_VENDOR_SDL=ON
cmake --build build --config Release
```
