# Dodge Game

## Tính năng

### Hai chế độ chơi:

- **Classic**: Né tránh chướng ngại vật càng lâu càng tốt, với độ khó tăng dần theo thời gian.
- **Survival Rush**: Sống sót trong 60 giây với mật độ chướng ngại vật cao hơn.

### Hiệu ứng thời tiết:

- **Mưa**: Giảm tốc độ người chơi 20% và hiển thị các giọt nước rơi.
- **Sương mù**: Giảm tầm nhìn bằng lớp phủ mờ trên màn hình.
- **Thay đổi ngẫu nhiên**:
  - Classic: Mỗi 30 giây (kéo dài 10 giây).
  - Survival Rush: Mỗi 10 giây (kéo dài 5 giây).

### Kỹ năng Flash:
- Dịch chuyển nhanh theo hướng chuột với thời gian hồi chiêu 15 giây.

### Lưu và tải game:
- Hỗ trợ lưu tiến trình trong chế độ **Classic**.

### Điểm cao:
- Lưu trữ và hiển thị điểm cao nhất cho cả hai chế độ.

---

## Cách chơi

### Điều khiển:

- **Di chuyển**: Sử dụng chuột để di chuyển nhân vật đến vị trí con trỏ.
- **Flash**: Nhấn phím `F` để dịch chuyển nhanh (hồi chiêu 15 giây).
- **Lưu game**: Nhấn phím `S` trong chế độ Classic để lưu và quay lại menu.

### Menu:

- Dùng phím **Lên/Xuống** để chọn tùy chọn.
- Nhấn **Enter** để xác nhận.

---

## Chế độ chơi

### Classic:

- Mục tiêu là né tránh chướng ngại vật càng lâu càng tốt.
- Mỗi 30 giây, một hiệu ứng thời tiết xuất hiện trong 10 giây.
- Điểm số tăng theo thời gian sống sót.

### Survival Rush:

- Sống sót trong 60 giây với mật độ chướng ngại vật cao.
- Mỗi 10 giây, một hiệu ứng thời tiết xuất hiện trong 5 giây.

---

## Hiệu ứng thời tiết

- **Mưa**: Giảm tốc độ người chơi và hiển thị giọt nước rơi.
- **Sương mù**: Lớp phủ mờ giảm tầm nhìn trên màn hình.

---

## Cấu trúc mã nguồn

Mã nguồn được tổ chức thành các file để dễ quản lý và bảo trì:

- `Constants.h`: Chứa các hằng số toàn cục như kích thước màn hình, tốc độ, và thời gian.
- `GameObject.h/cpp`: Quản lý chướng ngại vật và giọt nước (hiệu ứng mưa).
- `WeatherSystem.h/cpp`: Xử lý logic và hiển thị hiệu ứng thời tiết.
- `HighScore.h/cpp`: Quản lý lưu trữ và cập nhật điểm cao.
- `Utils.h/cpp`: Cung cấp các hàm tiện ích để hiển thị văn bản.
- `Game.h/cpp`: Lớp chính điều phối trò chơi, xử lý sự kiện, cập nhật trạng thái, và vẽ giao diện.
- `main.cpp`: Điểm vào chương trình, khởi tạo và chạy trò chơi.
