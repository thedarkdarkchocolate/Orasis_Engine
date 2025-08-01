#include "RenderPass.hpp"


/*

    I complicated implementation watch SwapChain::createRenderPass

*/

namespace Orasis {

    RenderPass::SubpassAttachment::SubpassAttachment
    (
        VkFormat attachFormat,
        uint8_t subpassToAttach,
        Attachment::Type type
    )
    :s_attachFormat{attachFormat}, s_subpassToAttach{subpassToAttach}, s_type{type}
    {
        // Trying to catch a misconfiguration
        if(s_previousSubpass > subpassToAttach && s_previousSubpass + 1 != subpassToAttach)
            throw std::runtime_error("Incorrect subpass to attach value");

        // Only incrementing the previous subpass if the current one is higher by one than the prevSubpass
        if(s_previousSubpass + 1 == subpassToAttach)
            s_previousSubpass++;

    }

    RenderPass::SubpassAttachment::SubpassAttachment
    (
        AttachmentInfo attachment
    )
    :s_attachFormat{attachment.s_format}, s_subpassToAttach{attachment.s_subpass}, s_type{attachment.s_type}
    {
        // Trying to catch a misconfiguration
        if(s_previousSubpass > s_subpassToAttach && s_previousSubpass + 1 != s_subpassToAttach)
            throw std::runtime_error("Incorrect subpass to attach value");

        // Only incrementing the previous subpass if the current one is higher by one than the prevSubpass
        if(s_previousSubpass + 1 == s_subpassToAttach)
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



    RenderPass::RenderPass(
        Device& device,
        std::unordered_map<uint8_t, std::vector<SubpassAttachment>> subPassAttachments,
        std::vector<SubpassAttachment> renderPassAttachmentsStruct,
        std::vector<VkSubpassDependency> subpassDependancies
    )
    : m_device{device}
    {
        // The s_previous here will hold the last subpass value so it hold the total 
        uint8_t totalSubpasses = (renderPassAttachmentsStruct[renderPassAttachmentsStruct.size() - 1].s_previousSubpass + 1);

        std::vector<VkAttachmentDescription> renderPassAttachments = {};
        
        // Configuring Attachements Description
        for (int i = 0; i < renderPassAttachmentsStruct.size(); i++)
        {
            SubpassAttachment currAttachment = renderPassAttachmentsStruct[i];
            VkAttachmentDescription desc{};

            desc.format           = currAttachment.s_attachFormat;
            desc.samples          = VK_SAMPLE_COUNT_1_BIT;
            desc.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
            desc.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;

            switch (currAttachment.s_type)
            {
                case Attachment::Type::isColor:
                    desc.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
                    desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    break;
                case Attachment::Type::isDepth:
                    desc.storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    break;
                case Attachment::Type::isPresented:
                    desc.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
                    desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    break;
                default:
                    throw std::runtime_error("Unknown attachment type!");
            }

            renderPassAttachments.push_back(desc);
        }

        renderPassAttachments.shrink_to_fit();

        // JUST A PLACE HOLDER IN CASE IN THE FUTURE I WANT TO IMPLEMENT
        // A RENDER PASS WHERE I CHOSE IF THE PREV ATTACHMENTS NEED TO BE PASSED AS INPUT 
        bool PREV_SUBPASS_ATTACHMENTS_AS_INPUT = true; 

        std::vector<VkSubpassDescription> subPasses;
        subPasses.resize(totalSubpasses);

        std::vector<std::vector<VkAttachmentReference>> subPassesRefs;
        subPassesRefs.resize(totalSubpasses);
        
        std::vector<std::vector<VkAttachmentReference>> inputSubpassRefs;
        inputSubpassRefs.resize(totalSubpasses);

        for (int i = 0; i < totalSubpasses; i++)
        {
            subPasses[i] = {};
            subPassesRefs[i] = {};
            inputSubpassRefs[i] = {};
        }
        
        VkAttachmentReference depthAttachmentRef{};

        for (int currSubpass = 0; currSubpass < totalSubpasses; currSubpass++)
        {

            bool hasDepthAttachment{false};
            bool needsPrevAttachmentsAsInput{false}; 

            for(int i = 0; i < subPassAttachments[currSubpass].size(); i++)
            {
                auto currSubPassAttachment = subPassAttachments[currSubpass][i];

                VkAttachmentReference ref{};
                
                switch (currSubPassAttachment.s_type)
                {

                    case Attachment::Type::isColor:
                    
                        ref.attachment = currSubPassAttachment.s_attachIndex;
                        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                        subPassesRefs[currSubpass].push_back(ref);
                        break;

                    case Attachment::Type::isDepth:

                        hasDepthAttachment = true;

                        depthAttachmentRef.attachment = currSubPassAttachment.s_attachIndex;
                        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        break;

                    case Attachment::Type::isPresented:

                        ref.attachment = currSubPassAttachment.s_attachIndex;
                        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                        subPassesRefs[currSubpass].push_back(ref);
                        break;

                    default:
                        throw std::runtime_error("Unknown attachment type!");
                }

            }

            if (currSubpass > 0 && PREV_SUBPASS_ATTACHMENTS_AS_INPUT)
            {
                needsPrevAttachmentsAsInput = true;

                for(int i = 0; i < subPassAttachments[currSubpass - 1].size(); i++)
                {
                    auto prevSubPassAttachment = subPassAttachments[currSubpass - 1][i];

                    if(prevSubPassAttachment.s_type == Attachment::Type::isColor)
                    {
                        VkAttachmentReference ref{};
                        ref.attachment = prevSubPassAttachment.s_attachIndex;
                        ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                        inputSubpassRefs[currSubpass].push_back(ref);
                    }

                }
                
            }
            else 
            {
                // Pushing back an empty ref so there are no alignment issues
                VkAttachmentReference ref{};
                inputSubpassRefs[currSubpass].push_back(ref);
            }

            subPasses[currSubpass] = {};
            subPasses[currSubpass].pipelineBindPoint            = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subPasses[currSubpass].colorAttachmentCount         = static_cast<uint32_t>(subPassesRefs[currSubpass].size());
            subPasses[currSubpass].pColorAttachments            = subPassesRefs[currSubpass].data();

            if (hasDepthAttachment)
                subPasses[currSubpass].pDepthStencilAttachment  = &depthAttachmentRef;
            
            if (needsPrevAttachmentsAsInput && !inputSubpassRefs[currSubpass].empty()){
                subPasses[currSubpass].inputAttachmentCount     = static_cast<uint32_t>(inputSubpassRefs[currSubpass].size());
                subPasses[currSubpass].pInputAttachments        = inputSubpassRefs[currSubpass].data();
            }


        }

        // if (subpassDependancies.size() != totalSubpasses)
        //     throw std::runtime_error("dependencies count doesn't match subpass count");
        
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount    = static_cast<uint32_t>(renderPassAttachments.size());
        renderPassInfo.pAttachments       = renderPassAttachments.data();
        renderPassInfo.subpassCount       = static_cast<uint32_t>(subPasses.size());
        renderPassInfo.pSubpasses         = subPasses.data();
        renderPassInfo.dependencyCount    = static_cast<uint32_t>(subpassDependancies.size());
        renderPassInfo.pDependencies      = subpassDependancies.data();

        if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }


    }


}