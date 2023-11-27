link để tìm hiểu cách kết nối mqtt bằng 4g "https://linhkienthuduc.com/huong-dan-su-dung-module-4g-voi-esp32-mqtt"
link tìm hiểu về MCU "https://thietbiketnoi.com/mcu-la-gi-vi-dieu-khien-tich-hop-tren-cac-he-thong-nhung.html"
link để vẽ lưu đồ thuật toán "https://app.diagrams.net/?fbclid=IwAR13zLX3UPSbVYh85KTmiexB3rFXzJEYI_FqM-Smb0GY6lSC_tfE-dbrjjk"
link để học về C+ "https://www.laptrinhdientu.com/"

Ý TƯỞNG

- Thiết bị nhận dữ liệu gps, mỗi 2s sẽ gửi lên web, nếu có ai quẹt thẻ ID thì dừng gửi gps, chụp ảnh lại và gửi lên web, vị trí, ảnh đã được mã hóa thành dạng text và ID thẻ quẹt
- mọi dữ liệu từ Web gửi về ứng dụng quản lý

KIẾN THỨC CẦN TÌM HIỂU

- đầu tiên cần có kiến thức về các ngoại vi của stm32 gồm UART, Timer, xem video này để có một số hình dung về kiến thức liên quan "https://www.youtube.com/watch?v=6kBSlSxHXJ0"
-  UART IRQ handler sẽ được gọi mỗi khi một byte dữ liệu được nhận từ UART, trong khi RXCPCallback sẽ được gọi khi toàn bộ dữ liệu được nhận từ UART. Việc sử dụng UART IRQ handler và RXCPCallback phụ thuộc vào từng ứng dụng cụ thể và yêu cầu của nó.
- sẽ dùng ngắt Timer để mỗi 2s gửi dữ liệu gps, 2s là thời gian đảm bảo tín hiệu truyền đi đáp ứng thời gian thực, tức là gần như bắt kịp mọi di chuyển của thiết bị.
- tìm hiểu về module GPS tại đây "https://docs.ai-thinker.com/en/gps%E6%A8%A1%E7%BB%84%E4%B8%93%E9%A2%982", decode chuỗi mã gửi từ module theo hình trong file gps.png, tìm hiểu về các định dạng gps "https://brandidowns.com/?p=77"
- thông tin về gps, thời gian truyền, mã ảnh cũng như ID đều được mã hóa thành chuỗi JSON "https://teky.edu.vn/blog/json-la-gi/"
- ESP32_CAM sử dụng để chụp ảnh cũng như truyền nhận dữ liệu từ MQTT broker "https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/"
ảnh chụp từ ESP32-CAM muốn gửi qua MQTT thì phải được mã hóa thành mã base64, khi debug, muốn chuyển ảnh sang base64 thì vào link "https://www.rapidtables.com/web/tools/base64-to-image.html"
- module RFID chúng ta sử dụng là loại module khi quẹt thẻ nó sẽ lấy ID thẻ và gửi về vdk qua giao thức UART, tìm hiểu về module ở đây "https://www.amazon.com/Taidacent-Wireless-Reader-Frequency-Wiegand/dp/B08QRCLS6L?th=1"
- link tìm các hàm cơ bản trong dùng string của c là "https://viettuts.vn/c-string/ham-strcat-trong-c"

TIẾN HÀNH CODE

* STM32F103C8T6
- Đầu tiên cấu hình chân trong CubeMx gồm SYS, TIM4, USART1(gps),USART2(esp32),USART3(rfid), Chú ý đưa mức ưu tiên của ngắt TIM4 lên mức 1, để đảm bảo việc gửi gps luôn được xét trước
- Về tổng quan code chô stm32 sẽ gồm các cờ để decode GPS, các cờ để xử lý trạng thái của thiết bị(truyền gps hay cả gps và rfid), hàm ngắt uart, ngắt timer, hàm tạo chuỗi json, hàm xóa chuỗi json, các hàm điều khiển trang thái led 12xanh,13do,14trang
, hàm truyền gps và hàm truyền gpsrfid
- sau phần khai báo các biến thì ta đi tìm hiểu cách decode dữ liệu gps trong file stm32f1xx_it.c, chúng ta sử dụng 3 flag làm dấu hiệu, flag1 giúp ta tìm ra định danh GNGGA, flag2 giúp ta ghi dữ liệu phía sau mã GNGGA, flag3 chỉ ra rằng chuỗi gps đã được đưa vào buffer
- trong file main.c ta tìm hiểu theo thứ tự như sau, hàm HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) sẽ dựa vào trạng thái esp32 gửi về và trạng thái của module RFID để thiết lập trạng thái: 1. truyền GPS
                                                              2. truyền RFIDGPS
                                                              3. lỗi kết nối  
- hàm HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) để thiết lập mỗi 2s gửi dữ liệu GPS. điều kiện của nó là dữ liệu gps được lấy đủ vào buffer(flag3=1), ko có ai quẹt thẻ(rfidok = 0) và esp32 kết nối ổn định(espok=1)
- hàm Data_GPS sẽ tạo 1 mảng kiểu json để chứa nội dung kinh độ, vĩ độ và thời gian kể từ khi khởi động thiết bị đến lúc gửi dữ liệu, tiếp đó sử dụng hàm strtok để tách chuỗi ra thành các mục và dựa vào ảnh gps.png ta lấy các mục cần thiết, tiếp đó ta xóa dữ liệu của chuỗi json cũ rồi thêm nội dung từ các mục đã tách được vào chuỗi json mới, thêm từ lớn đến bé để đảm bảo dữ liệu ko bị rối. Tại sao rối thì thử chèn từ bé đến lớn đi sẽ thấy :)))
- hàm Data_RFIDGPS cũng tương tự như hàm Data_GPS, chỉ khác có thêm dữ liệu ID, và thêm nội dung chuyển từ mã ascii sang hex của dữ liệu rfid, chuyển sang hex để ID nhìn đẹp.
- trong hàm clearjson thì xóa kiểu này để dữ liệu thiết lập về key of JSON ban đầu của mảng ko bị mất đi
- cuối cùng trong main thì khởi động ngắt uart 1, khởi động ngắt timer, cho nháy led 1 chút, vào while 1 thì chờ dữ liệu từ esp32 đến, kiểm tra trạng thái cờ và thực thi, nếu esp32 báo lỗi wifi thì cho nháy led và chờ phản hồi từ esp32, nếu esp32 báo đang truyền thì sáng led đỏ, nếu esp32 ok rồi thì chờ rfid thôi.
- quá trình làm phải debug và can thiệp vào từng ô của các mảng để thiết lập chèn và xóa cho đúng.

* ESP32 - CAM
- Về tổng quan thì chúng ta phải code cho esp32 kết nối mạng, kết nối mqtt, giao tiếp với stm32, chụp ảnh và mã hóa ảnh, sửa lại chuỗi json rồi gửi chuỗi json lên broker
- sau khi khai báo thư viện, ta khai báo wifi, thiết lập chân cho cam, truy cập địa chỉ Ip và lập hàm chỉnh chất lượng ảnh và kích thước ảnh
- quá trình chạy tôi thấy khi quẹt thẻ json hay bị lỗi kiểu {abcd{efgh} nên tôi thực hiện chỉnh sửa chuỗi json bằng một số hàm như  indexOf và substring...
- phần cuối là phần mã hóa ảnh sang base64 và giới hạn chiều dài mã base64, đây là phần lấy từ anh Thao