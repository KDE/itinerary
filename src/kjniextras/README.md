# KJniExtras

Utilities for a more type-safe use of the Java Native Interface (JNI),
built on top of Qt6's QJni* API.

Example:

``` c++
class Intent : public QtJniTypes::JObject<Intent>
{
    KJNI_OBJECT(Intent)
public:
    // default ctor
    KJNI_CONSTRUCTOR(Intent)
    // ctor with arguments
    KJNI_CONSTRUCTOR(Intent, QString)

    // methods, KJNI_STATIC_METHOD works in the same way for static methods
    KJNI_METHOD(Intent, addCategory, QString)

    // static properties (constants), KJNI_PROPERTY works in the same way for non-static properties
    KJNI_CONSTANT(QString, ACTION_VIEW)
};

KJNI_DECLARE_CLASS(android, content, Intent, Intent)
```

Usage:

``` c++
// properties appear as (static) members of the wrapper class,
// read/write access is internally re-routed to the corresponding JNI calls
Intent i(Intent::ACTION_VIEW);

// calling methods with the wrong argument types will fail
i.addCategory(u"foo"_s);

// arrays as arguments or properties also transparently work with this
QJniArray<Intent> a;
```
