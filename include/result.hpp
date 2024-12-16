#include <utility>
#include <stdexcept>

namespace eng {
    template<typename T>
    class result {
    public:
        static result<T> success(T value) { return result<T>(std::move(value)); }
        static result<T> error(const char* error_message) { return result<T>(std::move(error_message)); }

        result(const result& other) {
            successful = other.successful;  
            if (successful) {
                new (&value) T(other.value);
            }
            else {
                message = other.message;
            }
        }
        result(result&& other) noexcept(std::is_nothrow_move_constructible<T>::value) {
            successful = other.successful;
            if (successful) {
                new (&value) T(std::move(other.value));
            }
            else {
                message = other.message;
            }
        }

        ~result() {
            if (successful) {
                value.~T();
            }
        }

        result& operator=(const result& other) {
            if (this == &other) {
                return *this;
            }

            this->~result();

            successful = other.successful;
            if (successful) {
                new (&value) T(other.value);
            }
            else {
                message = other.message;
            }

            return *this;
        }

        result& operator=(result&& other) noexcept(std::is_nothrow_move_constructible<T>::value) {
            if (this == &other) {
                return *this;
            }

            this->~result();

            successful = other.successful;
            if (successful) {

                new (&value) T(other.value);
            }
            else {
                message = other.message;
            }

            return *this;
        }

        bool is_success() const {
            return success;
        }
        bool is_error() const {
            return !success;
        }

        const T& unwrap() const {
            if (!successful) {
                throw std::logic_error(std::string("Called unwrap on an error result: ") + message);
            }

            return value;
        }

        T& unwrap() {
            if (!successful) {
                throw std::logic_error(std::string("Called unwrap on an error result: ") + message);
            }

            return value;
        }

        const char* error_message() const {
            if (successful) {
                throw std::logic_error("Called error_message() on a successful result.");
            }

            return message;
        }

    private:
        explicit result(T value)
            : successful(true), value(std::move(value)) {}

        explicit result(const char* error_message)
            : successful(false), message(std::move(error_message)) {}

        bool successful;
        union {
            T value;
        };
        const char* message;
    };
}