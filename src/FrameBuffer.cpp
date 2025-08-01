#pragma once

#include "FrameBuffer.hpp"


namespace Orasis {


    // --------------- FrameBuffer Builder ---------------

    // FrameBuffer::Builder& FrameBuffer::Builder::addAttachment(AttachmentInfo attachInfo)
    // {
    //     attachments.emplace_back(std::move(attachInfo));
    //     return *this;
    // }
    
    // FrameBuffer::Builder& FrameBuffer::Builder::addAttachment(VkFormat format, VkImageUsageFlags usage, Attachment::Type type, uint8_t subpass)
    // {
    //     attachments.emplace_back(format, usage, type, subpass);
    //     return *this;
    // }
    
    // FrameBuffer::Builder& FrameBuffer::Builder::addDepthAttachment(AttachmentInfo attachInfo)
    // {
    //     if (attachInfo.s_type != Attachment::Type::isDepth)
    //         throw std::runtime_error("Attachment type is not deapth but used addDepthAttachment method");

    //     attachments.emplace_back(std::move(attachInfo));
    //     return *this;
    // }

    // FrameBuffer::Builder& FrameBuffer::Builder::addDepthAttachment(VkFormat format, uint8_t subpass)
    // {
    //     attachments.emplace_back(format, VkImageUsageFlags(), Attachment::Type::isDepth, subpass);
    //     return *this;
    // }

    // FrameBuffer::Builder& FrameBuffer::Builder::addDependancy(VkSubpassDependency dependency)
    // {
    //     dependancies.emplace_back(std::move(dependency));
    //     return *this;
    // }

    // -------------------- BUILDER END --------------------


}