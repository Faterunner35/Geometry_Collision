

#pragma once

#include "Matrix4x4.h"
#include "VisualProgram.h"
#include "ProgramFactory.h"
#include "ConstantBuffer.h"
#include "Texture.h"
#include "TextureArray.h"

namespace Vector_GM
{
    class VisualEffect
    {
    public:
        // Construction and destruction.
        virtual ~VisualEffect() = default;
        VisualEffect(std::shared_ptr<VisualProgram> const& program);

        // Member access.
        inline std::shared_ptr<VisualProgram> const& GetProgram() const
        {
            return mProgram;
        }

        inline std::shared_ptr<Shader> const& GetVertexShader() const
        {
            return mProgram->GetVertexShader();
        }

        inline std::shared_ptr<Shader> const& GetPixelShader() const
        {
            return mProgram->GetPixelShader();
        }

        inline std::shared_ptr<Shader> const& GetGeometryShader() const
        {
            return mProgram->GetGeometryShader();
        }

        // For convenience, provide a projection-view-world constant buffer
        // that an effect can use if so desired.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer);

        inline std::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const
        {
            return mPVWMatrixConstant;
        }

        inline void SetPVWMatrix(Matrix4x4<float> const& pvwMatrix)
        {
            *mPVWMatrixConstant->Get<Matrix4x4<float>>() = pvwMatrix;
        }

        inline Matrix4x4<float> const& GetPVWMatrix() const
        {
            return *mPVWMatrixConstant->Get<Matrix4x4<float>>();
        }

    protected:
        // For derived classes to defer construction because they want to
        // create programs via a factory.
        VisualEffect();

        std::shared_ptr<VisualProgram> mProgram;
        BufferUpdater mBufferUpdater;
        TextureUpdater mTextureUpdater;
        TextureArrayUpdater mTextureArrayUpdater;

        // The constant buffer that stores the 4x4 projection-view-world
        // transformation for the Visual object to which this effect is
        // attached.
        std::shared_ptr<ConstantBuffer> mPVWMatrixConstant;
    };
}
