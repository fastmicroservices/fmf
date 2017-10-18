#include <iostream>
#include <string>
#include <set>
#include <fstream>
#include <numeric>
#include <endpoint.hpp>
#include <plustache/context.hpp>
#include <plustache/template.hpp>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

std::string load(std::string const &file_name) {
    std::ifstream ifs(file_name);
    std::string content( std::istreambuf_iterator<char>(ifs), {});
                        //  std::istreambuf_iterator<char>() );
    return content;
}

struct Actor {
    static const int WIDTH = 200;
    static const int LEFT_MARGIN = 20;
    static const int RIGHT_MARGIN = 20;
    static const int TOTAL_WIDTH = (WIDTH + LEFT_MARGIN + RIGHT_MARGIN);
    static const int HEIGHT = 50;
    static const int TOP_MARGIN = 5;
    static const int BOTTOM_MARGIN = 5;
    static const int TOTAL_HEIGHT = (HEIGHT + TOP_MARGIN + BOTTOM_MARGIN);
    static const int LABEL_MARGIN = 3;
    static const int LABEL_MARGIN_Y = 2;
    static const int LABEL_MARGIN_TEXT_Y = 15;

    std::string _name;
    int _level, _order;
    Actor(std::string const &name) :_name(name) {}

    int left() const {
        return _level * TOTAL_WIDTH + LEFT_MARGIN;
    }
    int right() const {
        return (_level + 1) * TOTAL_WIDTH - RIGHT_MARGIN;
    }
    int top() const {
        return _order * TOTAL_HEIGHT + TOP_MARGIN;
    }
    int middle() const {
        return _order * TOTAL_HEIGHT + TOTAL_HEIGHT / 2;
    }
};

struct Action 
{
    Actor *_from, *_to;
    std::string _description;
    std::string explain() {
        return _from->_name + " " + _description + " " + _to->_name;
    }
};

typedef std::set<std::shared_ptr<Actor>> ActorSet;

Actor *introduce_actor_at_level(std::string const &actor_name, ActorSet &coll, int level) {
    auto a = std::make_shared<Actor>(actor_name);
    a->_level = level;
    auto max_at_level = std::accumulate(coll.begin(), coll.end(), 0, [level](int last_max, std::shared_ptr<Actor> const &candidate) {
        return (candidate->_level == level && candidate->_order > last_max) ? candidate->_order : last_max;
    });
    a->_order = max_at_level + 1;
    coll.emplace(a);
    return a.get();
}


std::string process_graph(std::string const &payload) {
    static auto actor_idiom = load("graph/idioms/actor.mustache");
    static auto svg_idiom = load("graph/idioms/svg.mustache");
    static auto action_idiom = load("graph/idioms/action.mustache");
    if (payload.empty()) {
        return std::string("We need a JSON document");
    }
    // interpret the json
    rapidjson::Document doc;
    doc.Parse(payload.c_str());
    auto name = doc["name"].GetString();
    ActorSet actors;
    std::vector<std::shared_ptr<Action>> actions;
    for (auto &scenario: doc["scenarios"].GetArray()) {
        for (auto &step: scenario.GetArray()) {
            auto action = std::make_shared<Action>();
            action->_description = step["action"].GetString();
            auto actor_name = step["actor"].GetString();
            auto actor_pos = std::find_if(actors.begin(), actors.end(), [actor_name](std::shared_ptr<Actor> const &a) { return a->_name == actor_name; });
            if (actor_pos == actors.end()) {
                // this is a new actor, created ex machina
                action->_from = introduce_actor_at_level(actor_name, actors, 0);
            }
            else {
                // this is an actor we know about
                action->_from = actor_pos->get();
            }
            // now find the object, or introduce it as a new actor
            auto object_name = step["object"].GetString();
            auto object_pos = std::find_if(actors.begin(), actors.end(), [object_name](std::shared_ptr<Actor> const &a) { return a->_name == object_name; });
            if (object_pos == actors.end()) {
                // introduce, to the right
                action->_to = introduce_actor_at_level(object_name, actors, action->_from->_level + 1);
            } else
            {
                action->_to = object_pos->get();
            }
            actions.push_back(action);
        }
    }
    
    static Plustache::template_t templ;
    Plustache::Context mdata;
    mdata.add("text", name);
    std::string contents;
    int max_level = 0, max_order = 0;
    // const int ACTOR_LABEL_WIDTH = ACTOR_WIDTH - 2 * ACTOR_LABEL_MARGIN;
    PlustacheTypes::ObjectType actor {
        {"width", std::to_string(Actor::WIDTH)},
        {"height", std::to_string(Actor::HEIGHT)}
    };
    for (auto const &actor_to_render: actors) {
        actor["name"] = actor_to_render->_name;
        auto actor_left = actor_to_render->left();
        actor["left"] = std::to_string(actor_left);
        auto actor_top = actor_to_render->top();
        actor["top"] = std::to_string(actor_top);
        actor["label_left"] = std::to_string(actor_left + Actor::LABEL_MARGIN);
        actor["label_top"] = std::to_string(actor_top + Actor::TOTAL_HEIGHT - Actor::LABEL_MARGIN_TEXT_Y);
        contents += templ.render(actor_idiom, actor);
        max_level = actor_to_render->_level > max_level ? actor_to_render->_level : max_level;
        max_order = actor_to_render->_order > max_order ? actor_to_render->_order : max_order;
    }
    auto label_top = (Actor::TOTAL_HEIGHT * (max_order + 1));
    PlustacheTypes::ObjectType action {
        {"span", "5s"},
        {"label_left", std::to_string(Actor::LABEL_MARGIN)},
        {"label_top", std::to_string(label_top)}
    };
    int action_number = 0;
    for (auto const &action_to_render: actions) {
        action["number"] = std::to_string(action_number);
        action["description"] = action_to_render->explain();
        action["from_actor"] = action_to_render->_from->_name;
        action["to_actor"] = action_to_render->_to->_name;
        auto start = 1 + action_number * 7;
        action["start"] = std::to_string(start) + "s";
        action["start_offset"] = std::to_string(start + 1) + "s";
        action["from_y"] = std::to_string(action_to_render->_from->middle());
        action["to_y"] = std::to_string(action_to_render->_to->middle());
        if (action_to_render->_to->_level > action_to_render->_from->_level) {
            action["from_x"] = std::to_string(action_to_render->_from->right());
            action["to_x"] = std::to_string(action_to_render->_to->left());
        }
        else {
            action["from_x"] = std::to_string(action_to_render->_from->left());
            action["to_x"] = std::to_string(action_to_render->_to->right());
        }
        contents += templ.render(action_idiom, action);
        action_number++;
    }
    mdata.add("contents", contents);
    mdata.add("canvas_width", std::to_string(Actor::TOTAL_WIDTH * ++max_level));
    mdata.add("canvas_height", std::to_string(Actor::TOTAL_HEIGHT * ++max_order));
    return templ.render(svg_idiom, mdata);
}

void service_main()
{
    std::cout << "OK working as a service" << std::endl;
    auto config = FMF::ConfigurationFactory::create("env,inmem");
    auto endpoint = FMF::BindingEndpointFactory::create("http", config);
    
    endpoint->handle_topic("graph", 0, 1, [](std::string const &payload, FMF::Context &ctx) {
        ctx.set("Content-Type", "image/svg+xml");
        return process_graph(payload);
    });
    endpoint->handle_topic("graph.test", 0, 1, [](std::string const &payload, FMF::Context &ctx) {
        ctx.set("Content-Type", "text/html");
        ctx.set("Serve-File", "graph/web/test.html");
        return std::string();
    });
    while (endpoint->listen()) {;}
}

int main(int argc, char *argv[]) 
{
    if (argc > 1) {
        std::cout << process_graph(load(argv[1]));
    }
    else {
        service_main();
    }
    return 0;
}