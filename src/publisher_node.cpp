#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>
#include <ncurses.h>

class KeyboardPublisher : public rclcpp::Node {
public:
    KeyboardPublisher() : Node("keyboard_publisher") {
        publisher_ = this->create_publisher<std_msgs::msg::Int32>("/motor_control", 10);
        initscr();
        raw();
        keypad(stdscr, TRUE);
        noecho();
        listenForKeyPress();
    }

    ~KeyboardPublisher() {
        endwin();
    }

    void listenForKeyPress() {
        int ch;
        while (rclcpp::ok()) {
            ch = getch();
            switch (ch) {
                case KEY_UP:
                    publishCommand(1); // forward
                    RCLCPP_INFO(this->get_logger(), "Moving Forward");
                    break;
                case KEY_DOWN:
                    publishCommand(2); // backward
                    RCLCPP_INFO(this->get_logger(), "Moving Backward");
                    break;
                case KEY_LEFT:
                    publishCommand(3); // left
                    RCLCPP_INFO(this->get_logger(), "Turning Left");
                    break;
                case KEY_RIGHT:
                    publishCommand(4); // right
                    RCLCPP_INFO(this->get_logger(), "Turning Right");
                    break;
                case 27: // ESC key
                    publishCommand(0); // stop
                    RCLCPP_INFO(this->get_logger(), "Stopping");
                    return;
                default:
                    break;
            }
        }
    }

    void publishCommand(int command) {
        auto msg = std_msgs::msg::Int32();
        msg.data = command;
        publisher_->publish(msg);
        RCLCPP_INFO(this->get_logger(), "Publishing: '%d'", command);
    }

private:
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr publisher_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<KeyboardPublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
