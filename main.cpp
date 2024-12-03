#include <iostream>
#include <queue>
#include <thread>
#include <chrono>
#include <mutex>
#include <SFML/Graphics.hpp>

// Traffic Light States
enum LightState
{
    RED,
    YELLOW,
    GREEN
};

// Traffic Light Class
class TrafficLight
{
private:
    LightState state;
    int greenTime, yellowTime, redTime;

public:
    TrafficLight(int g, int y, int r) : greenTime(g), yellowTime(y), redTime(r), state(RED) {}

    void changeState()
    {
        if (state == RED)
            state = GREEN;
        else if (state == GREEN)
            state = YELLOW;
        else
            state = RED;
    }

    LightState getState() { return state; }

    int getDuration()
    {
        if (state == GREEN)
            return greenTime;
        else if (state == YELLOW)
            return yellowTime;
        else
            return redTime;
    }

    sf::Color getColor()
    {
        if (state == GREEN)
            return sf::Color::Green;
        else if (state == YELLOW)
            return sf::Color::Yellow;
        else
            return sf::Color::Red;
    }
};

// Intersection Class with Emergency Vehicle Handling
class Intersection
{
private:
    std::queue<int> normalQueue;             // Queue for normal vehicles
    std::priority_queue<int> emergencyQueue; // Priority queue for emergency vehicles
    TrafficLight trafficLight;
    std::mutex mtx;

public:
    Intersection(int green, int yellow, int red) : trafficLight(green, yellow, red) {}

    // Add a vehicle to the appropriate queue
    void addVehicle(int vehicleId, bool isEmergency)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (isEmergency)
        {
            emergencyQueue.push(vehicleId);
            std::cout << "Emergency vehicle " << vehicleId << " added to the queue.\n";
        }
        else
        {
            normalQueue.push(vehicleId);
            std::cout << "Normal vehicle " << vehicleId << " added to the queue.\n";
        }
    }

    // Process the traffic signal and allow vehicles to pass
    void processSignal()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(trafficLight.getDuration()));
            std::lock_guard<std::mutex> lock(mtx);

            if (trafficLight.getState() == GREEN)
            {
                if (!emergencyQueue.empty())
                {
                    std::cout << "Emergency vehicle " << emergencyQueue.top() << " is passing.\n";
                    emergencyQueue.pop();
                }
                else if (!normalQueue.empty())
                {
                    std::cout << "Normal vehicle " << normalQueue.front() << " is passing.\n";
                    normalQueue.pop();
                }
            }

            trafficLight.changeState();
            std::cout << "Traffic light changed to "
                      << (trafficLight.getState() == GREEN ? "GREEN" : trafficLight.getState() == YELLOW ? "YELLOW"
                                                                                                         : "RED")
                      << ".\n";
        }
    }

    TrafficLight &getTrafficLight() { return trafficLight; }
};

// Graphics Visualization
void renderTrafficSystem(Intersection &intersection)
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Smart Traffic System");

    sf::RectangleShape light(sf::Vector2f(50, 150));
    light.setPosition(375, 200);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.clear();
        light.setFillColor(intersection.getTrafficLight().getColor());
        window.draw(light);
        window.display();
    }
}

// Main Function
int main()
{
    Intersection intersection(5, 2, 3);

    // Thread to simulate adding vehicles to the queue
    std::thread vehicleAdder([&]()
                             {
        for (int i = 1; i <= 10; ++i) {
            bool isEmergency = (i % 3 == 0); // Every 3rd vehicle is an emergency vehicle
            intersection.addVehicle(i, isEmergency);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } });

    // Thread to process traffic signals
    std::thread signalProcessor(&Intersection::processSignal, &intersection);

    // Thread to render the traffic system
    std::thread renderThread(renderTrafficSystem, std::ref(intersection));

    vehicleAdder.join();
    signalProcessor.join();
    renderThread.join();

    return 0;
}
