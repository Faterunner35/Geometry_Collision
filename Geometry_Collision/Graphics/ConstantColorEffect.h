

#pragma once

#include "VisualEffect.h"

namespace Vector_GM
{
    class ConstantColorEffect : public VisualEffect
    {
    public:
        // Construction.
        ConstantColorEffect(std::shared_ptr<ProgramFactory> const& factory, Vector4<float> const& color);

        // Member access.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& pvwMatrix) override;

        inline std::shared_ptr<ConstantBuffer> const& GetColorConstant() const
        {
            return mColorConstant;
        }

    private:
        // Vertex shader parameter.
        std::shared_ptr<ConstantBuffer> mColorConstant;

        // Shader source code as strings.
        static std::string const msGLSLVSSource;
        static std::string const msGLSLPSSource;
        static std::string const msHLSLVSSource;
        static std::string const msHLSLPSSource;
        static ProgramSources const msVSSource;
        static ProgramSources const msPSSource;
    };
}
