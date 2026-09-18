# ImgSorting

이미지를 세로 100조각으로 섞은 뒤 정렬 알고리즘 38종으로 차례로 복원하는 시각화. C++20, SDL3.

## 화면

- 정렬 창: 헤더에 알고리즘 이름, 비교·교환·쓰기 횟수, 경과 시간, 시간복잡도. 비교는 초록, 교환·쓰기는 빨강 막대
- 컨트롤 창: Import Image, Import Audio, Fit, 이전·다음, 정렬 이름 칸(누르면 전체 목록), Start, Stop, Restart, Shuffle
- 정렬 하나를 10초 동안 재생. 완료 시 음원 전체를 한 번 재생한 뒤 다음 정렬로 넘어감
- 끝나지 않는 정렬(Bozo, Bogobogo, Bogo)은 10초 뒤 포기. 마지막(Bogo) 뒤 종료

## 리소스

- 기본 이미지: 코드가 만드는 색상 그라디언트
- 기본 음원: `assets/default.wav`(합성음). 교환 중에는 첫 음을 겹쳐 재생하고, 완료 시 전체를 재생
- 직접 고른 이미지(PNG·JPG·BMP)·음원(WAV)은 Import 버튼으로 교체. 최근 임포트는 앱 데이터 폴더에 복사해 다음 실행 때 다시 불러옴
- 선택: `Images/stalin.jpeg`가 있으면 스탈린 정렬이 제거한 칸에 그 사진 조각을 표시. 저장소에는 없음

## 빌드 (macOS)

```bash
brew install cmake sdl3
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/sort_check
./build/ImgSorting [이미지 경로]
```

## Podman

이미지 빌드. 빌드 과정에서 정렬 검증(`sort_check`)까지 실행함.

```bash
podman build -t imgsorting .
```

Linux 데스크톱(X11, PulseAudio 또는 PipeWire-Pulse)에서 실행.

```bash
podman run --rm --userns=keep-id --security-opt label=disable --device /dev/dri \
  -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e PULSE_SERVER=unix:/tmp/pulse-native -v "$XDG_RUNTIME_DIR/pulse/native:/tmp/pulse-native" \
  -e XDG_DATA_HOME=/data -v imgsorting-data:/data \
  imgsorting
```

- Wayland: `-e WAYLAND_DISPLAY -e XDG_RUNTIME_DIR=/tmp/xdg -v "$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY:/tmp/xdg/$WAYLAND_DISPLAY"` 추가
- 이미지 지정: `-v ~/Pictures:/pictures:ro` 추가 후 맨 끝에 `/pictures/파일.png`
- `imgsorting-data` 볼륨: 최근 임포트 보존용. 빼면 매번 기본 리소스로 시작
- macOS는 컨테이너에서 화면·소리 연결이 번거로워 네이티브 빌드 권장

## 구성

- `src/main.cpp`: 창 2개, 재생, 효과음, 컨트롤
- `src/sorts.h`, `src/sorts.cpp`: 알고리즘 표와 호출
- `src/sorts/*.cpp`: 정렬 구현. 비교·교환·쓰기를 기록해 재생
- `tests/sort_check.cpp`: 모든 정렬의 결과·연산 인덱스 검증
- `third_party/stb_image.h`: stb_image v2.30(public domain, nothings/stb `2c980bb`)
