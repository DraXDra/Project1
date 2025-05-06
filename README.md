# Dodge Game

## Lối chơi & Logic của Game

**Dodge Game** là trò chơi né tránh, trong đó người chơi điều khiển nhân vật để tránh các chướng ngại vật rơi từ bốn phía (trên, dưới, trái, phải). Mục tiêu là sống sót càng lâu càng tốt.

### Logic chính:
- Di chuyển nhân vật mượt mà theo vị trí con trỏ chuột.
- Tạo chướng ngại vật ngẫu nhiên với tốc độ và tần suất tăng dần.
- Áp dụng hiệu ứng thời tiết để tăng thử thách.
- Quản lý các trạng thái trò chơi và chế độ chơi khác nhau.

---

## Chướng Ngại Vật

- Hình dạng: Hình vuông (30x30 pixel).
- Vị trí xuất hiện: Ngẫu nhiên ngoài màn hình (trên, dưới, trái, phải).
- Vận tốc: Di chuyển thẳng vào màn hình.
- Tốc độ: Bắt đầu từ 3 pixel/frame, tăng dần trong chế độ Classic.
- Texture: Dùng ảnh `assets/obstacle.png`.

---

## Các chế độ chơi

### Classic:
- **Mục tiêu**: Sống sót càng lâu càng tốt.
- **Tần suất chướng ngại vật**: Bắt đầu từ 0.5 giây, giảm mỗi 30 giây (tối thiểu 0.1 giây).
- **Tốc độ chướng ngại vật**: Tăng 0.5 mỗi 10 giây.
- **Hiệu ứng thời tiết**: Xuất hiện mỗi 30 giây, kéo dài 10 giây.

### Survival Rush:
- **Mục tiêu**: Sống sót 60 giây.
- **Tần suất chướng ngại vật**: 0.2 giây.
- **Tốc độ**: Không thay đổi.
- **Hiệu ứng thời tiết**: Thay đổi mỗi 10 giây, kéo dài 5 giây.

---

## Trạng Thái Trò Chơi

1. **MENU**: Hiển thị menu chính (Play Classic, Survival Rush, Load Game, Exit).
2. **PLAYING**: Chơi chế độ Classic.
3. **PLAYING_SURVIVAL**: Chơi chế độ Survival Rush.
4. **GAME_OVER**: Khi va chạm hoặc hết thời gian, hiển thị điểm số và tùy chọn quay lại menu.

---

## Logic Thắng/Thua

### Thua:
- Va chạm với chướng ngại vật.
- Trong Survival Rush: Hết thời gian mà chưa sống sót.

### Thắng:
- **Classic**: Không có kết thúc, chơi đến khi thua.
- **Survival Rush**: Sống sót hết 60 giây → ghi nhận điểm cao.

---

## Tăng Độ Khó

### Classic:
- Tăng tốc độ chướng ngại vật 0.5 mỗi 10 giây.
- Giảm thời gian tạo chướng ngại vật 50ms mỗi 30 giây (tối thiểu 100ms).
- Thêm hiệu ứng thời tiết (mưa, sương mù).

### Survival Rush:
- Tần suất cao hơn ngay từ đầu (0.2 giây).
- Hiệu ứng thời tiết thay đổi nhanh hơn (mỗi 10 giây).

---

## Âm Thanh

- **Nhạc nền**: `assets/background_music.mp3`, phát liên tục.
- **Hiệu ứng va chạm**: `assets/hit.mp3`, phát khi va chạm.
- **Quản lý âm thanh**: Sử dụng SDL_mixer, mỗi âm thanh phát ở kênh riêng.

---

## Hệ Thống Điểm

- **Tăng điểm**: 1 giây sống sót = 10 điểm.
- **Highscore**:
  - Classic: Lưu vào `highscore_classic.txt`.
  - Survival Rush: Lưu vào `highscore_survivalrush.txt`.
- **Hiển thị**: Trong menu, cập nhật sau mỗi game.

---

## Các Thành Phần Chính

- **Nhân vật**: Hình vuông 50x50 pixel, di chuyển theo chuột, có kỹ năng **Flash**:
  - Dịch chuyển 200 pixel theo hướng chuột.
  - Hồi chiêu: 15 giây.
- **Chướng ngại vật**: Xuất hiện từ 4 phía, tốc độ tăng theo thời gian (Classic).
- **Hiệu ứng thời tiết**:
  - **Mưa**: Giảm tốc độ người chơi 20%, hiển thị giọt nước.
  - **Sương mù**: Lớp phủ mờ, giảm tầm nhìn.
- **Giao diện**:
  - Hiển thị điểm số, thời tiết, hồi chiêu Flash, thời gian còn lại (Survival Rush).

---

## Hành Vi Chướng Ngại Vật

- **Xuất hiện**: Ngẫu nhiên từ 1 trong 4 phía.
- **Di chuyển**: Theo hướng vào màn hình, tốc độ tùy chế độ.
- **Xóa khỏi bộ nhớ**: Khi vượt khỏi màn hình.

---

## Kiểm Tra Va Chạm

- So sánh hình chữ nhật bao quanh:
  - Nhân vật: 50x50 pixel.
  - Chướng ngại vật: 30x30 pixel.
- **Nếu giao nhau → Game Over**.

---

## Vòng Lặp Trò Chơi

Thực hiện trong `Game::run`:

1. **Xử lý sự kiện**:
   - Chuột: Di chuyển.
   - Bàn phím: Flash, lưu game, menu.

2. **Cập nhật trạng thái**:
   - Nhân vật di chuyển theo chuột.
   - Tạo & cập nhật chướng ngại vật.
   - Cập nhật thời tiết.
   - Kiểm tra va chạm & thời gian sống.

3. **Vẽ giao diện**:
   - Nền, nhân vật, chướng ngại vật.
   - Hiệu ứng thời tiết.
   - Văn bản (điểm, hồi chiêu, thời gian còn lại).

4. **Giới hạn tốc độ khung hình**:
   - Delay 16ms mỗi frame (~60 FPS).

---

## Công Cụ & Nguồn Tham Khảo

### Công cụ:
- **SDL2**: Đồ họa, âm thanh, đầu vào.
- **SDL_image**: Tải hình ảnh PNG.
- **SDL_mixer**: Quản lý âm thanh.
- **SDL_ttf**: Hiển thị văn bản.

### Tài nguyên & Tham khảo:
- [Tài liệu SDL2](https://wiki.libsdl.org/)
- [Hướng dẫn lập trình game với SDL]
- [GitHub - LoL Dodge Game](https://github.com/jaingmengmeng/LoL-Dodge-Game/tree/master)
- [GitHub - Dodge Game SDL](https://github.com/Hert4/dodge_game/tree/main)
- ChatGPT hỗ trợ viết logic, tạo hình ảnh và sửa lỗi.
- Tài nguyên đồ họa/âm thanh: Tự tạo hoặc nguồn miễn phí.
