#ifndef __QUEUE_FUNCTION_H__
#define __QUEUE_FUNCTION_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <functional>
#include <type_traits>

// Define which controls whether the postFunc is called when the mainFunc is not invoked.
// #define QUEUE_FUNCTION_ENABLE_POST_FUNC_CALL_WHEN_MAIN_NOT_INVOKED

// This header file uses the standard namespace.
using namespace std;

// This struct represents the return value type of the user-specified main function.
template <typename T> struct RValue {
    using type = T;
};

// Declare the QueueFunction within our DispatchCPP namespace.
namespace DispatchCPP {
    // This class allows for representing a pre, main, and post function with dynamically specifying the return value and
    // arguments to the functions, themselves. Only a main func is required -- specifying pre/post functions is optional.
    template <class RType, typename ...Args> class QueueFunction {
        public:

            // =========================================================================================================

            function<void(void)>                            initFunc;

            function<bool(Args...)>                         preFunc;

            function<typename RValue<RType>::type(Args...)> mainFuncNotVoid;
            function<void(Args...)>                         mainFuncVoid;

            void *                                          pPostFuncNotVoid;
            void *                                          pPostFuncVoid;

            function<void(void)>                            closeFunc;

            // =========================================================================================================

            QueueFunction(function<typename RValue<RType>::type(Args...)> newMainFunc,
                          function<bool(Args...)>                         newPreFunc   = nullptr,
                          void *                                          pNewPostFunc = nullptr,
                          function<void(void)>                            newInitFunc  = nullptr,
                          function<void(void)>                            newCloseFunc = nullptr) {
                this->setMainFunc(newMainFunc);
                this->setPreFunc(newPreFunc);
                this->setPostFunc(pNewPostFunc);
                this->setInitFunc(newInitFunc);
                this->setCloseFunc(newCloseFunc);
            };

            virtual ~QueueFunction() {
                this->initFunc         = nullptr;
                this->mainFuncNotVoid  = nullptr;
                this->mainFuncVoid     = nullptr;
                this->preFunc          = nullptr;
                this->pPostFuncNotVoid = nullptr;
                this->pPostFuncVoid    = nullptr;
                this->closeFunc        = nullptr;
            };

            // =========================================================================================================

            void setPreFunc(function<bool(Args...)> newPreFunc)  {
                this->preFunc = newPreFunc;
            };

            // --------------------

            template<typename Q = RType>
            typename enable_if<!is_same<Q, void>::value, void>::type setMainFunc(function<typename RValue<RType>::type(Args...)> newMainFunc) {
                this->mainFuncNotVoid = newMainFunc;
            };

            template<typename Q = RType>
            typename enable_if<is_same<Q, void>::value, void>::type setMainFunc(function<void(Args...)> newMainFunc) {
                this->mainFuncVoid = newMainFunc;
            };

            // --------------------

            template<typename Q = RType>
            typename enable_if<!is_same<Q, void>::value, void>::type setPostFunc(void * pNewPostFunc) {
                this->pPostFuncNotVoid = ((function<void(typename RValue<RType>::type)> *) pNewPostFunc);
            };

            template<typename Q = RType>
            typename enable_if<is_same<Q, void>::value, void>::type setPostFunc(void * pNewPostFunc) {
                this->pPostFuncVoid = ((function<void(void)> *) pNewPostFunc);
            };

            // --------------------

            void setInitFunc(function<void(void)> newInitFunc) {
                this->initFunc = newInitFunc;
            };

            void setCloseFunc(function<void(void)> newCloseFunc) {
                this->closeFunc = newCloseFunc;
            }

            // =========================================================================================================

            bool runPreFunc(Args... args) {
                return(this->preFunc(args...));
            };

            // ---------------------

            template<typename Q = RType>
            typename enable_if<!is_same<Q, void>::value, typename RValue<RType>::type>::type runMainFunc(Args... args) {
                return(this->mainFuncNotVoid(args...));
            };

            template<typename Q = RType>
            typename enable_if<is_same<Q, void>::value, void>::type runMainFunc(Args... args) {
                this->mainFuncVoid(args...);
            };

            // =========================================================================================================

            void runInitFunctions(void) {
                if (this->initFunc != nullptr) {
                    this->initFunc();
                }
            };

            template<typename Q = RType>
            typename enable_if<!is_same<Q, void>::value, void>::type runFunctions(Args... args) {
                bool preFuncResult = true;
                if ((this->preFunc != nullptr) && (this->mainFuncNotVoid != nullptr)) {
                    preFuncResult = this->runPreFunc(args...);
                }
                if (preFuncResult && (this->mainFuncNotVoid != nullptr)) {
                    typename RValue<RType>::type mainFuncNotVoidResult = this->runMainFunc(args...);
                    if (this->pPostFuncNotVoid != nullptr) {
                        function<void(typename RValue<RType>::type)> func = *((function<void(typename RValue<RType>::type)> *) this->pPostFuncNotVoid);
                        func(mainFuncNotVoidResult);
                    }
#ifdef QUEUE_FUNCTION_ENABLE_POST_FUNC_CALL_WHEN_MAIN_NOT_INVOKED
                } else {
                    typename RValue<RType>::type uninitializedmainFuncNotVoidResult{};
                    this->runPostFunc(uninitializedmainFuncNotVoidResult);
#endif // QUEUE_FUNCTION_ENABLE_POST_FUNC_CALL_WHEN_MAIN_NOT_INVOKED
                }
            };

            template<typename Q = RType>
            typename enable_if<is_same<Q, void>::value, void>::type runFunctions(Args... args) {
                bool preFuncResult = true;
                if ((this->preFunc != nullptr) && (this->mainFuncNotVoid != nullptr)) {
                    preFuncResult = this->runPreFunc(args...);
                }
                if (preFuncResult && (this->mainFuncVoid != nullptr)) {
                    this->runMainFunc(args...);
                    if (this->pPostFuncVoid != nullptr) {
                        function<void(void)> func = *((function<void(void)> *) this->pPostFuncVoid);
                        func();
                    }
#ifdef QUEUE_FUNCTION_ENABLE_POST_FUNC_CALL_WHEN_MAIN_NOT_INVOKED
                } else {
                    this->runPostFunc();
#endif // QUEUE_FUNCTION_ENABLE_POST_FUNC_CALL_WHEN_MAIN_NOT_INVOKED
                }
            };

            // =========================================================================================================

            // Working example of std::enable_if to enable/disable two different functions depending upon an incoming template param:

            // Is enabled when the QueueFunction has return value that IS void:
            template<typename Q = RType>
            typename enable_if<is_same<Q, void>::value, bool>::type hasReturnValue() {
                return(false);
            };

            // Is enabled when the QueueFunction has a return value that's NOT void:
            template<typename Q = RType>
            typename enable_if<!is_same<Q, void>::value, bool>::type hasReturnValue() {
                return(true);
            };
    };
};

#endif // __QUEUE_FUNCTION_H__
