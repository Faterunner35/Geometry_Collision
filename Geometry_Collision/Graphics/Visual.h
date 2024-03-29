
#pragma once

#include "Spatial.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VisualEffect.h"

namespace Vector_GM
{
    class Visual : public Spatial
    {
    public:
        // Construction and destruction.
        virtual ~Visual() = default;
        Visual(
            std::shared_ptr<VertexBuffer> const& vbuffer = std::shared_ptr<VertexBuffer>(),
            std::shared_ptr<IndexBuffer> const& ibuffer = std::shared_ptr<IndexBuffer>(),
            std::shared_ptr<VisualEffect> const& effect = std::shared_ptr<VisualEffect>());

        // Member access.
        inline void SetVertexBuffer(std::shared_ptr<VertexBuffer> const& vbuffer)
        {
            mVBuffer = vbuffer;
        }

        inline void SetIndexBuffer(std::shared_ptr<IndexBuffer> const& ibuffer)
        {
            mIBuffer = ibuffer;
        }

        inline void SetEffect(std::shared_ptr<VisualEffect> const& effect)
        {
            mEffect = effect;
        }

        inline std::shared_ptr<VertexBuffer> const& GetVertexBuffer() const
        {
            return mVBuffer;
        }

        inline std::shared_ptr<IndexBuffer> const& GetIndexBuffer() const
        {
            return mIBuffer;
        }

        inline std::shared_ptr<VisualEffect> const& GetEffect() const
        {
            return mEffect;
        }

        // Support for geometric updates.
        bool UpdateModelBound();
        bool UpdateModelNormals();

        // Public member access.
        BoundingSphere<float> modelBound;

    protected:
        // Support for geometric updates.
        inline virtual void UpdateWorldBound() override
        {
            modelBound.TransformBy(worldTransform, worldBound);
        }

        // Support for hierarchical culling.
        inline virtual void GetVisibleSet(Culler& culler,
            std::shared_ptr<Camera> const& camera, bool noCull) override
        {
            (void)camera;
            (void)noCull;
            culler.Insert(this);
        }

        std::shared_ptr<VertexBuffer> mVBuffer;
        std::shared_ptr<IndexBuffer> mIBuffer;
        std::shared_ptr<VisualEffect> mEffect;
    };
}
