#ifndef FACE_OBSERVATION_SNAPSHOT_H
#define FACE_OBSERVATION_SNAPSHOT_H

#include <cstdlib>
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"
#include "../rapidjson/prettywriter.h"

using namespace rapidjson;

namespace moface {

    struct FaceObservationSnapShot {
        int64 timestamp;
        float pitch;
        float roll;
        float yaw;
        std::vector<::mediapipe::NormalizedLandmarkList> landmarks;
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
            void Serialize(Writer& writer) const {
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
            ObservationFeed(int64 timestamp) : _timestamp(timestamp), _tracked_positions() {}
            ObservationFeed& operator=(const ObservationFeed& rhs) {
                _timestamp = rhs._timestamp;
                _tracked_positions = rhs._tracked_positions;
                return *this;
            }
            ~ObservationFeed() {}

            void AddTrackedPosition(const ObservationTrackedPosition& tracked_position) {
                _tracked_positions.push_back(tracked_position);
            }
            template <typename Writer>
            void Serialize(Writer& writer) const {
                writer.StartObject();
                writer.String("timestamp");
                writer.Int64(_timestamp);
                writer.StartArray();
                for (
                    std::vector<ObservationTrackedPosition>::const_iterator pos_itr = _tracked_positions.begin();
                    pos_itr != _tracked_positions.end();
                    ++pos_itr
                )
                    pos_itr->Serialize(writer);
                writer.EndArray();
                writer.EndObject();
            }
        private:
            int64 _timestamp;
            std::vector<ObservationTrackedPosition> _tracked_positions;
    };

    class ObservationRoi {
        public:
            ObservationRoi(double x, double y, double width, double height) : _x(x), _y(y), _width(width), _height(height) {}
            ~ObservationRoi() {}
            template <typename Writer>
            void Serialize(Writer& writer) const {
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
            ~ObservationAction() { delete _roi; }
            template <typename Writer>
            void Serialize(Writer& writer) const {
                writer.StartObject();
                writer.String("type");
                #if RAPIDJSON_HAS_STDSTRING
                writer.String(_type);
                #else
                writer.String(_type.c_str(), static_cast<SizeType>(_type.length()));
                #endif
                _roi->Serialize(writer);
                writer.StartArray();
                for (
                    std::vector<ObservationFeed>::const_iterator feed_itr = _feeds.begin();
                    feed_itr != _feeds.end();
                    ++feed_itr
                )
                    feed_itr->Serialize(writer);
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
            template <typename Writer>
            void Serialize(Writer& writer) const {
                writer.StartObject();
                writer.String("name");
                #if RAPIDJSON_HAS_STDSTRING
                writer.String(_name);
                #else
                writer.String(_name.c_str(), static_cast<SizeType>(_name.length()));
                #endif
                writer.StartArray();
                for (
                    std::vector<ObservationAction>::const_iterator action_itr = _actions.begin();
                    action_itr != _actions.end();
                    ++action_itr
                )
                    action_itr->Serialize(writer);
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

            void update_media_file_path(const std::string& file_path) {
                _file_path = file_path;
                return;
            }
            template <typename Writer>
            void Serialize(Writer& writer) const {
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
                    objectItr->Serialize(writer);
                writer.EndArray();
                writer.EndObject();
            }
        private:
            std::string _file_path;
            std::vector<ObservationObject> _objects;
    };

}

#endif
