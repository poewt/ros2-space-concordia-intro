#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "build_my_string/msg/message.hpp"

using namespace std::chrono_literals;

class MessagePublisher : public rclcpp::Node
{
  public:
    MessagePublisher() :
      Node("message_publisher"),
      count_(0),
      base_width(20),
      base_height(40),
      playerPos(0),
      playerDirection(RIGHT),
      curr_msg_pos(0),
      loop(0),
      eyes('o')
    {
      // Initialize custom parameters
      this->declare_parameter("message", "Hi!");
      this->declare_parameter("base_w", base_width);
      this->declare_parameter("base_h", base_height);

      cached_base_width = base_width;
      cached_base_height = base_height;
      cached_inp_msg = "Hi!";

      // Initialize base layer
      set_base_width();

      // Create publisher
      publisher_ = this->create_publisher<build_my_string::msg::Message>("message_publisher", 10);

      // Create repeating loop that calls timer_callback() every x ms
      timer_ = this->create_wall_timer(
        100ms, std::bind(&MessagePublisher::timer_callback, this)
      );
    }

  private:
    // Function called on repeat by timer_
    void timer_callback()
    {
      // Get parameters
      this->get_parameter("message", input_message);
      this->get_parameter("base_w", base_width);
      this->get_parameter("base_h", base_height);

      // Check to see if param still matches cached string
      if (input_message != cached_inp_msg) {
        cached_inp_msg = input_message;
        reset();
      }
      if (base_width != cached_base_width) {
        cached_base_width = base_width;
        base_layer = "";
        set_base_width();
        reset();
      }
      if (base_height != cached_base_height) {
        cached_base_height = base_height;
        reset();
      }

      output_message.clear();

      if (curr_msg_pos < input_message.length()) {        // Print output
        print_output();
      } else {
        // celebrate
        celebrate();
      }
      

      // Set message and publish
      auto user_msg = build_my_string::msg::Message();
      user_msg.message = output_message;
      publisher_->publish(user_msg);


      RCLCPP_INFO(this->get_logger(), user_msg.message.c_str());


      // Increase current count
      this->count_++;
    }
    
    // Publisher variables
    rclcpp::Publisher<build_my_string::msg::Message>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

    // todo Add variables to keep track of build progress
    enum PlayerDirection {
      LEFT,
      RIGHT
    };

    size_t count_;

    // base layer
    int base_width;
    int base_height;
    std::string base_layer;
    int cached_base_width;
    int cached_base_height;

    // player layer
    std::string player_layer;
    size_t playerPos;
    PlayerDirection playerDirection;

    // message input
    std::string input_message;
    std::string cached_inp_msg;
    std::string output_message;
    size_t curr_msg_pos;

    // celebration
    size_t loop;
    char eyes;

    // todo Function to build output
    void print_output()
    {
      // Print base layer
      print_base();

      // Print player
      print_player();

      output_message += "Building: ["+input_message+"].\n";
    }

    // Function to print player layer
    void print_player()
    {
      // Set new current layer
      std::string curr_layer = player_layer;

      // Increment based on direction player is moving
      if (playerDirection == RIGHT) {
        playerPos++;
        curr_layer[playerPos] = '(';
        curr_layer[playerPos + 1] = '>';
        curr_layer[playerPos + 2] = 'o';
        curr_layer[playerPos + 3] = '_';
        curr_layer[playerPos + 4] = 'o';
        curr_layer[playerPos + 5] = ')';
        curr_layer[playerPos + 6] = '>';
        // player holds the letter
        curr_layer[playerPos + 7] = input_message[curr_msg_pos]; 
      } else if (playerDirection == LEFT) {
        playerPos--;
        curr_layer[playerPos] = '<';
        curr_layer[playerPos + 1] = '(';
        curr_layer[playerPos + 2] = 'o';
        curr_layer[playerPos + 3] = '_';
        curr_layer[playerPos + 4] = 'o';
        curr_layer[playerPos + 5] = '<';
        curr_layer[playerPos + 6] = ')';
      }

      // Once player reaches far end of the base, reverse direction
      if (playerPos == player_layer.length() - 8) {
        playerDirection = LEFT;
        // Append next character in message and continue with next in line
        player_layer += input_message[curr_msg_pos]; 
        curr_msg_pos++;
      } else if (playerPos == 0) {
        playerDirection = RIGHT;
      }

      output_message += curr_layer + '\n';
    }

    // Function to print base layer
    void print_base()
    {
      for (int i = 0; i < base_height; i++)
      {
        output_message += base_layer;
      }
    }

    // Function to set base width
    void set_base_width()
    {
      for (int i = 0; i < base_width; i++)
      {
        base_layer += ".";
      }
      player_layer = base_layer;
      base_layer += '\n';
    }

    // Function to celebrate after completing string
    void celebrate()
    {
      print_base();
      print_dance();
    }

    // Function to print dance animation
    void print_dance()
    {
      // Set current animation layer
      std::string dance_layer = player_layer;

      // Increment based on direction player is moving
      if (playerDirection == RIGHT) {
        playerPos++;
        dance_layer[playerPos] = '(';
        dance_layer[playerPos + 1] = '\\';
        dance_layer[playerPos + 2] = eyes;
        dance_layer[playerPos + 3] = '_';
        dance_layer[playerPos + 4] = eyes;
        dance_layer[playerPos + 5] = ')';
        dance_layer[playerPos + 6] = '/';
      } else if (playerDirection == LEFT) {
        playerPos--;
        dance_layer[playerPos] = '\\';
        dance_layer[playerPos + 1] = '(';
        dance_layer[playerPos + 2] = eyes;
        dance_layer[playerPos + 3] = '_';
        dance_layer[playerPos + 4] = eyes;
        dance_layer[playerPos + 5] = '/';
        dance_layer[playerPos + 6] = ')';
      }

      // Once player reaches far end of the base, reverse direction
      if (playerPos == player_layer.length() - 7 - input_message.length()) {
        playerDirection = LEFT;
        loop++;
      } else if (playerPos == 0) {
        playerDirection = RIGHT;
        loop++;
      }

      // Interchange eyes based on loop time
      if (loop % 10 == 0) {
        eyes = 'o';
      } else if (loop % 5 == 0) {
        eyes = '^';
      } else if (loop % 3 == 0) {
        eyes = 'w';
      }

      // RCLCPP_INFO(this->get_logger(), dance_layer.c_str());

      // Set message to be published
      // auto user_msg = build_my_string::msg::Message();
      // user_msg.message = dance_layer;
      // publisher_->publish(user_msg);

      // user_msg.message = "I finished building your string [" + input_message + "]. Dance party!"; 
      // publisher_->publish(user_msg);

      output_message += dance_layer + '\n';
      output_message += "I finished building your string [" + input_message + "]. Dance party!"; 
      
      // RCLCPP_INFO(this->get_logger(), "I finished building your string ['%s']. Dance party!  [frame: %zu]", user_msg.message.c_str(), this->count_);
    }

    void reset()
    {
        count_ = 0;
        curr_msg_pos = 0;
        player_layer = base_layer;
        playerPos = 0;
        playerDirection = RIGHT;
    }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MessagePublisher>());
  rclcpp::shutdown();
  return 0;
}


