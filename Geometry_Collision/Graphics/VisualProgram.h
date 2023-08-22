

#pragma once

#include "Shader.h"

namespace Vector_GM
{
    class VisualProgram
    {
    public:
        // DX11 uses the class as is.  GL45 derives from the class to store
        // the shader and program handles.
        virtual ~VisualProgram() = default;
        VisualProgram() = default;

        // Member access.
        inline std::shared_ptr<Shader> const& GetVertexShader() const
        {
            return mVertexShader;
        }

        inline std::shared_ptr<Shader> const& GetPixelShader() const
        {
            return mPixelShader;
        }

        inline std::shared_ptr<Shader> const& GetGeometryShader() const
        {
            return mGeometryShader;
        }

        inline void SetVertexShader(std::shared_ptr<Shader> const& shader)
        {
            if (shader)
            {
                LogAssert(shader->GetType() == GT_VERTEX_SHADER, "The input must be a vertex shader.");
            }
            mVertexShader = shader;
        }

        inline void SetPixelShader(std::shared_ptr<Shader> const& shader)
        {
            if (shader)
            {
                LogAssert(shader->GetType() == GT_PIXEL_SHADER, "The input must be a pixel shader.");
            }
            mPixelShader = shader;
        }

        inline void SetGeometryShader(std::shared_ptr<Shader> const& shader)
        {
            if (shader)
            {
                LogAssert(shader->GetType() == GT_GEOMETRY_SHADER, "The input must be a geometry shader.");
            }
            mGeometryShader = shader;
        }

    private:
        std::shared_ptr<Shader> mVertexShader;
        std::shared_ptr<Shader> mPixelShader;
        std::shared_ptr<Shader> mGeometryShader;
    };
}
