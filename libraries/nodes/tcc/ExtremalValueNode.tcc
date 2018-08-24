////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Learning Library (ELL)
//  File:     ExtremalValueNode.tcc (nodes)
//  Authors:  Chuck Jacobs
//
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ell
{
namespace nodes
{
    template <typename ValueType, bool max>
    ExtremalValueNode<ValueType, max>::ExtremalValueNode()
        : CompilableNode({ &_input }, { &_val, &_argVal }), _input(this, {}, inputPortName), _val(this, valPortName, 1), _argVal(this, argValPortName, 1)
    {
    }

    template <typename ValueType, bool max>
    ExtremalValueNode<ValueType, max>::ExtremalValueNode(const model::PortElements<ValueType>& input)
        : CompilableNode({ &_input }, { &_val, &_argVal }), _input(this, input, inputPortName), _val(this, valPortName, 1), _argVal(this, argValPortName, 1)
    {
    }

    template <typename ValueType, bool max>
    std::string ExtremalValueNode<ValueType, max>::GetTypeName()
    {
        if (max)
        {
            return utilities::GetCompositeTypeName<ValueType, std::true_type>("ExtremalValueNode");
        }
        else
        {
            return utilities::GetCompositeTypeName<ValueType, std::false_type>("ExtremalValueNode");
        }
    }

    template <typename ValueType, bool max>
    void ExtremalValueNode<ValueType, max>::Compute() const
    {
        auto inputValues = _input.GetValue();
        decltype(std::max_element(inputValues.begin(), inputValues.end())) result;
        if (max)
        {
            result = std::max_element(inputValues.begin(), inputValues.end());
        }
        else
        {
            result = std::min_element(inputValues.begin(), inputValues.end());
        }
        auto val = *result;
        auto index = result - inputValues.begin();
        _val.SetOutput({ val });
        _argVal.SetOutput({ (int)index });
    };

    template <typename ValueType, bool max>
    emitters::TypedComparison ExtremalValueNode<ValueType, max>::GetComparison() const
    {
        if (IsMaxNode())
        {
            return emitters::GetComparison<ValueType>(emitters::BinaryPredicateType::greater);
        }
        else
        {
            return emitters::GetComparison<ValueType>(emitters::BinaryPredicateType::less);
        }
    }

    template <typename ValueType, bool max>
    void ExtremalValueNode<ValueType, max>::Compile(model::IRMapCompiler& compiler, emitters::IRFunctionEmitter& function)
    {
        VerifyIsScalar(val);
        VerifyIsScalar(argVal);
        if (IsPureVector(input) && !compiler.GetCompilerOptions().unrollLoops)
        {
            CompileLoop(compiler, function);
        }
        else
        {
            CompileExpanded(compiler, function);
        }
    }

    template <typename ValueType, bool max>
    void ExtremalValueNode<ValueType, max>::CompileLoop(model::IRMapCompiler& compiler, emitters::IRFunctionEmitter& function)
    {
        emitters::LLVMValue inputVal = compiler.EnsurePortEmitted(input);
        emitters::LLVMValue outVal = compiler.EnsurePortEmitted(val);
        emitters::LLVMValue outArgVal = compiler.EnsurePortEmitted(argVal);
        auto inputType = GetPortVariableType(input);
        auto numInputs = input.Size();

        emitters::LLVMValue bestVal = function.Variable(inputType, "bestVal");
        emitters::LLVMValue bestIndex = function.Variable(ell::emitters::VariableType::Int32, "bestArgVal");

        auto val0 = function.ValueAt(inputVal, function.Literal(0));
        function.Store(bestVal, val0);
        function.StoreZero(bestIndex);

        function.For(1, numInputs, 1, [inputVal, bestVal, bestIndex, this](emitters::IRFunctionEmitter& function, emitters::LLVMValue i){
            auto val = function.ValueAt(inputVal, i);
            function.If(GetComparison(), val, function.Load(bestVal), [bestVal, bestIndex, val, i](auto& function) {
                function.Store(bestVal, val);
                function.Store(bestIndex, i);
            });
        });

        function.Store(outVal, function.Load(bestVal));
        function.Store(outArgVal, function.Load(bestIndex));
    }

    template <typename ValueType, bool max>
    void ExtremalValueNode<ValueType, max>::CompileExpanded(model::IRMapCompiler& compiler, emitters::IRFunctionEmitter& function)
    {
        emitters::LLVMValue outVal = compiler.EnsurePortEmitted(val);
        emitters::LLVMValue outArgVal = compiler.EnsurePortEmitted(argVal);
        auto inputType = GetPortVariableType(input);
        auto numInputs = input.Size();

        emitters::LLVMValue bestVal = function.Variable(inputType, "bestVal");
        emitters::LLVMValue bestIndex = function.Variable(ell::emitters::VariableType::Int32, "bestArgVal");

        emitters::LLVMValue val0 = compiler.LoadPortElementVariable(input.GetInputElement(0));
        function.Store(bestVal, val0);
        function.StoreZero(bestIndex);

        for (size_t i = 1; i < numInputs; ++i)
        {
            emitters::LLVMValue val = compiler.LoadPortElementVariable(input.GetInputElement(i));
            function.If(GetComparison(), val, function.Load(bestVal), [bestVal, bestIndex, val, i](auto& function) {
                function.Store(bestVal, val);
                function.Store(bestIndex, function.Literal(static_cast<int>(i)));
            });
        }

        function.Store(outVal, function.Load(bestVal));
        function.Store(outArgVal, function.Load(bestIndex));
    }

    template <typename ValueType, bool max>
    void ExtremalValueNode<ValueType, max>::WriteToArchive(utilities::Archiver& archiver) const
    {
        Node::WriteToArchive(archiver);
        archiver[inputPortName] << _input;
        archiver[valPortName] << _val;
        archiver[argValPortName] << _argVal;
    }

    template <typename ValueType, bool max>
    void ExtremalValueNode<ValueType, max>::ReadFromArchive(utilities::Unarchiver& archiver)
    {
        Node::ReadFromArchive(archiver);
        archiver[inputPortName] >> _input;
        archiver[valPortName] >> _val;
        archiver[argValPortName] >> _argVal;
    }

    //
    // Copy definitions for subclasses
    //
    template <typename ValueType>
    void ArgMinNode<ValueType>::Copy(model::ModelTransformer& transformer) const
    {
        auto newPortElements = transformer.TransformPortElements(this->_input.GetPortElements());
        auto newNode = transformer.AddNode<ArgMinNode<ValueType>>(newPortElements);
        transformer.MapNodeOutput(this->val, newNode->val);
        transformer.MapNodeOutput(this->argVal, newNode->argVal);
    }

    template <typename ValueType>
    void ArgMaxNode<ValueType>::Copy(model::ModelTransformer& transformer) const
    {
        auto newPortElements = transformer.TransformPortElements(this->_input.GetPortElements());
        auto newNode = transformer.AddNode<ArgMaxNode<ValueType>>(newPortElements);
        transformer.MapNodeOutput(this->val, newNode->val);
        transformer.MapNodeOutput(this->argVal, newNode->argVal);
    }
}
}
