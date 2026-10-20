#pragma once

template <typename T>
class Emitter {
public:
    virtual ~Emitter() = default;
    virtual void emit(const T& item) = 0;
};

template <typename I, typename O>
class Stage {
public:
    virtual ~Stage() = default;
    virtual void process(const I& input, Emitter<O>& out) = 0;
    virtual void open() {} // Yaşam döngüsü kancası
    virtual void close() {}
};

template <typename O>
class Source {
public:
    virtual ~Source() = default;
    virtual void produce(Emitter<O>& out) = 0;
};

template <typename I>
class Sink {
public:
    virtual ~Sink() = default;
    virtual void consume(const I& item) = 0;
};