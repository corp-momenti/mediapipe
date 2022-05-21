#ifndef FACE_OBSERVATION_SNAPSHOT_H
#define FACE_OBSERVATION_SNAPSHOT_H

#include <cstdlib>
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"
#include "../rapidjson/prettywriter.h"

using namespace rapidjson;

namespace moface {

    struct FaceObservationSnapShot {
        double timestamp;
        int frame_id;
        float pitch;
        float roll;
        float yaw;
        std::vector<::mediapipe::NormalizedLandmarkList> landmarks;
    };

    class ObservationRotation {
        public:
            ObservationRotation(float pitch, float yaw, float roll) : _pitch(pitch), _yaw(yaw), _roll(roll) {}
            ObservationRotation& operator=(const ObservationRotation& rhs) {
                _pitch = rhs._pitch;
                _yaw = rhs._yaw;
                _roll = rhs._roll;
                return *this;
            }
            ~ObservationRotation() {}
            template <typename Writer>
            void serialize(Writer& writer) const {
                writer.StartObject();
                writer.String("pitch");
                writer.Double(_pitch);
                writer.String("yaw");
                writer.Double(_yaw);
                writer.String("roll");
                writer.Double(_roll);
                writer.EndObject();
            }
        private:
            double _pitch, _yaw, _roll;
    };

    class ObservationTrackedPosition {
        public:
            ObservationTrackedPosition(float x, float y, float z) : _x(x), _y(y), _z(z) {}
            ObservationTrackedPosition& operator=(const ObservationTrackedPosition& rhs) {
                _x = rhs._x;
                _y = rhs._y;
                _z = rhs._z;
                return *this;
            }
            ~ObservationTrackedPosition() {}
            template <typename Writer>
            void serialize(Writer& writer) const {
                writer.StartObject();
                writer.String("x");
                writer.Double(_x);
                writer.String("y");
                writer.Double(_y);
                writer.String("z");
                writer.Double(_z);
                writer.EndObject();
            }
        private:
            double _x, _y, _z;
    };

    class ObservationFeed {
        public:
            ObservationFeed(
                double timestamp, double pitch, double yaw, double roll
            ) : _timestamp(timestamp), _rotation(new ObservationRotation(pitch, yaw, roll)), _tracked_positions() {}
            ObservationFeed& operator=(const ObservationFeed& rhs) {
                _timestamp = rhs._timestamp;
                _tracked_positions = rhs._tracked_positions;
                return *this;
            }
            ~ObservationFeed() {}
            void addTrackedPosition(const ObservationTrackedPosition& tracked_position) {
                _tracked_positions.push_back(tracked_position);
            }
            // void addRotation(ObservationRotation& rotation) {
            //     _rotation = std::addressof(rotation);
            // }
            template <typename Writer>
            void serialize(Writer& writer) const {
                writer.StartObject();
                writer.String("timestamp");
                writer.Double(_timestamp);
                writer.String("rotation");
                _rotation->serialize(writer);
                writer.String("tracked_position");
                writer.StartArray();
                for (
                    std::vector<ObservationTrackedPosition>::const_iterator pos_itr = _tracked_positions.begin();
                    pos_itr != _tracked_positions.end();
                    ++pos_itr
                )
                    pos_itr->serialize(writer);
                writer.EndArray();
                writer.EndObject();
            }
        private:
            double _timestamp;
            ObservationRotation *_rotation;
            std::vector<ObservationTrackedPosition> _tracked_positions;
    };

    class ObservationRoi {
        public:
            ObservationRoi(double x, double y, double width, double height) : _x(x), _y(y), _width(width), _height(height) {}
            ~ObservationRoi() {}
            template <typename Writer>
            void serialize(Writer& writer) const {
                writer.StartObject();
                writer.String("x");
                writer.Double(_x);
                writer.String("y");
                writer.Double(_y);
                writer.String("width");
                writer.Double(_width);
                writer.String("Height");
                writer.Double(_height);
                writer.EndObject();
            }
        private:
            double _x, _y, _width, _height;
    };

    class ObservationAction {
        public:
            ObservationAction(
                std::string type, double x, double y, double width, double height
            ) : _type(type), _roi(new ObservationRoi(x, y, width, height)) {}
            ObservationAction& operator=(const ObservationAction& rhs) {
                _type = rhs._type;
                _roi = rhs._roi;
                _feeds = rhs._feeds;
                return *this;
            }
            void addFeed(const ObservationFeed& feed) {
                _feeds.push_back(feed);
            }
            ~ObservationAction() { /*delete _roi;*/ /* why is this making a crash when adding an action to an object??? */}
            template <typename Writer>
            void serialize(Writer& writer) const {
                writer.StartObject();
                writer.String("type");
                #if RAPIDJSON_HAS_STDSTRING
                writer.String(_type);
                #else
                writer.String(_type.c_str(), static_cast<SizeType>(_type.length()));
                #endif
                writer.String("roi");
                _roi->serialize(writer);
                writer.String("feeds");
                writer.StartArray();
                for (
                    std::vector<ObservationFeed>::const_iterator feed_itr = _feeds.begin();
                    feed_itr != _feeds.end();
                    ++feed_itr
                )
                    feed_itr->serialize(writer);
                writer.EndArray();
                writer.EndObject();
            }
        private:
            std::string _type;
            ObservationRoi *_roi;
            std::vector<ObservationFeed> _feeds;
    };

    class ObservationObject {
        public:
            ObservationObject(std::string name) : _name(name) {}
            ObservationObject& operator=(const ObservationObject& rhs) {
                _name = rhs._name;
                _actions = rhs._actions;
                return *this;
            }
            ~ObservationObject() {}
            void addAction(const ObservationAction& action) {
                _actions.push_back(action);
            }
            template <typename Writer>
            void serialize(Writer& writer) const {
                writer.StartObject();
                writer.String("name");
                #if RAPIDJSON_HAS_STDSTRING
                writer.String(_name);
                #else
                writer.String(_name.c_str(), static_cast<SizeType>(_name.length()));
                #endif
                writer.String("actions");
                writer.StartArray();
                for (
                    std::vector<ObservationAction>::const_iterator action_itr = _actions.begin();
                    action_itr != _actions.end();
                    ++action_itr
                )
                    action_itr->serialize(writer);
                writer.EndArray();
                writer.EndObject();
            }
        private:
            std::string _name;
            std::vector<ObservationAction> _actions;
    };

    class FaceObservation {
        public:
            FaceObservation(const std::string& file_path) : _file_path(file_path), _objects() {}
            ~FaceObservation() {}

            FaceObservation& operator = (const FaceObservation& rhs) {
                _file_path = rhs._file_path;
                _objects = rhs._objects;
                return *this;
            }
            void updateMediaFilePath(const std::string& file_path) {
                _file_path = file_path;
                return;
            }
            void addObject(const ObservationObject& object) {
                _objects.push_back(object);
            }
            int objectCount() {
                return _objects.size();
            }
            ObservationObject& lastObject() {
                return _objects.back();
            }
            template <typename Writer>
            void serialize(Writer& writer) const {
                writer.StartObject();
                writer.String("file_path");
                #if RAPIDJSON_HAS_STDSTRING
                writer.String(_file_path);
                #else
                writer.String(_file_path.c_str(), static_cast<SizeType>(_file_path.length()));
                #endif
                writer.String("objects");
                writer.StartArray();
                for (
                    std::vector<ObservationObject>::const_iterator objectItr = _objects.begin();
                    objectItr != _objects.end();
                    ++objectItr
                )
                    objectItr->serialize(writer);
                writer.EndArray();
                writer.EndObject();
            }
        private:
            std::string _file_path;
            std::vector<ObservationObject> _objects;
    };

}

#endif
