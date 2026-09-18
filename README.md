# ImgSorting

재미로 만든 이미지 정렬 시각화. 이미지를 세로 100조각으로 섞은 뒤 정렬 알고리즘 38종으로 복원함.

## 실행

[Releases](https://github.com/FMsongX2/ImgSorting/releases)에서 받아 실행.

- Windows: `ImgSorting-windows-x64.zip`
- macOS: `ImgSorting-macos-universal.zip`. 실행이 막히면 `xattr -d com.apple.quarantine ImgSorting`
- Linux: `ImgSorting-linux-x64.tar.gz`

Controls 창에서 이미지·오디오(WAV·M4A) 불러오기, 시작·정지, 정렬 선택. Linux에서 M4A는 ffmpeg 필요.

## 정렬 38종 (재생 순서)

Bubble, Cocktail Shaker, Odd-Even, Gnome, Comb, Selection, Cycle, Insertion, Pancake, Merge, Block Merge(WikiSort), In-Place Merge, Tim, Patience, Strand, Quick, PDQ(Pattern-defeating Quicksort), Tournament, Tree, Heap, Intro, Smooth, Shell, Counting, Pigeonhole, Bucket, Flash, Radix(MSD), Radix(in-place, 이진 MSD), Radix(LSD), Gravity, Bitonic, Stooge, Slow, Stalin, Bozo, Bogobogo, Bogo

Bozo·Bogobogo·Bogo는 끝나지 않아 10초 뒤 포기함. 마지막 Bogo 뒤 종료.

General을 켜면 Bubble, Selection, Insertion, Merge, Quick, Heap, Shell, Counting, Bucket, Radix(LSD), Stalin, Bogo만 목록·재생에 남김.

## Podman (Linux)

저장소를 받은 뒤 그 폴더(`CMakeLists.txt`가 있는 곳)에서 실행.

```bash
git clone https://github.com/FMsongX2/ImgSorting.git
cd ImgSorting
podman build -t imgsorting .
podman run --rm --userns=keep-id --security-opt label=disable -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix imgsorting
```

## 빌드

위와 같은 저장소 폴더에서 실행.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DIMGSORTING_VENDOR_SDL=ON
cmake --build build --config Release
```
