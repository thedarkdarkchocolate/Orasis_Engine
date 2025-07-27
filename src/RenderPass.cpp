#include "RenderPass.hpp"


/*

    I complicated implementation watch SwapChain::createRenderPass

*/

namespace Orasis {

    RenderPass::SubpassAttachment::SubpassAttachment
    (
        VkAttachmentDescription attchDescr,
        VkFormat attachFormat,
        uint8_t subpassToAttach,
        bool isDeapthAttach
    )
    :s_attchDescr{attchDescr}, s_attachFormat{attachFormat}, s_subpassToAttach{subpassToAttach}, s_isDeapthAttach{isDeapthAttach}
    {
        // Trying to catch a misconfiguration
        if(s_previousSubpass > subpassToAttach || s_previousSubpass + 1 != subpassToAttach)
            throw std::runtime_error("Incorrect subpass to attach value");

        // Only incrementing the previous subpass if the current one is higher by one than the prevSubpass
        if(s_previousSubpass + 1 == subpassToAttach)
            s_previousSubpass++;
    }

    RenderPass::Builder& RenderPass::Builder::addSubpassAttachments(SubpassAttachment subAttachment)
    {

        subAttachment.s_attachIndex = attachmentNum++;
        s_subPassAttachments[subAttachment.s_subpassToAttach].emplace_back(subAttachment);
        s_renderPassAttachmentsStruct.emplace_back(subAttachment);
        return *this;
    }
    
    RenderPass::Builder& RenderPass::Builder::addSubpassDependency(VkSubpassDependency dependency)
    {
        s_dependencies.push_back(dependency);
        return *this;
    }

    std::unique_ptr<RenderPass> RenderPass::Builder::build() const
    {
        if(s_renderPassAttachmentsStruct.size() == 0)
            throw std::runtime_error("You need to attachments to create a Render Pass!");
        return std::make_unique<RenderPass>(s_device, s_subPassAttachments, s_renderPassAttachmentsStruct, s_dependencies);
    }






}