#pragma once

#include "aurora_engine/core/aurora_device.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace aurora {
    class AuroraDescriptorSetLayout {
        public:
            class Builder {
                public:
                    Builder(AuroraDevice &auroraDevice) : auroraDevice{auroraDevice} {}

                    Builder &addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
                    std::unique_ptr<AuroraDescriptorSetLayout> build() const;

                private:
                    AuroraDevice &auroraDevice;
                    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
            };

            AuroraDescriptorSetLayout(AuroraDevice &auroraDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
            ~AuroraDescriptorSetLayout();
            
            AuroraDescriptorSetLayout(const AuroraDescriptorSetLayout &) = delete;
            AuroraDescriptorSetLayout &operator=(const AuroraDescriptorSetLayout &) = delete;

            VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

        private:
            AuroraDevice &auroraDevice;
            VkDescriptorSetLayout descriptorSetLayout;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

            friend class AuroraDescriptorWriter;
    };

    class AuroraDescriptorPool {
        public:
            class Builder {
                public:
                    Builder(AuroraDevice &auroraDevice) : auroraDevice{auroraDevice} {}

                    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
                    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
                    Builder &setMaxSets(uint32_t count);
                    std::unique_ptr<AuroraDescriptorPool> build() const;

                private:
                    AuroraDevice &auroraDevice;
                    std::vector<VkDescriptorPoolSize> poolSizes{};
                    uint32_t maxSets = 1000;
                    VkDescriptorPoolCreateFlags poolFlags = 0;
            };

            AuroraDescriptorPool(AuroraDevice &auroraDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes);
            ~AuroraDescriptorPool();

            AuroraDescriptorPool(const AuroraDescriptorPool &) = delete;
            AuroraDescriptorPool &operator=(const AuroraDescriptorPool &) = delete;

            bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

            void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

            void resetPool();

        private:
            AuroraDevice &auroraDevice;
            VkDescriptorPool descriptorPool;

            friend class AuroraDescriptorWriter;
    };

    class AuroraDescriptorWriter {
        public:
            AuroraDescriptorWriter(AuroraDescriptorSetLayout &setLayout, AuroraDescriptorPool &pool);

            AuroraDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
            AuroraDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

            bool build(VkDescriptorSet &set);
            void overwrite(VkDescriptorSet &set);

        private:
            AuroraDescriptorSetLayout &setLayout;
            AuroraDescriptorPool &pool;
            std::vector<VkWriteDescriptorSet> writes;
    };
}