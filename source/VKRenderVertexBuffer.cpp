#include "vulkanPCH.h"

VKRenderVertexBuffer::VKRenderVertexBuffer()
{
	Buffer = 0;
	Memory = 0;

}

VKRenderVertexBuffer::~VKRenderVertexBuffer()
{
}
inline bool Test(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {

	for (uint32_t i = 0; i < Factory->PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((Factory->PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}

void VKRenderVertexBuffer::Create(bsize stride, bsize count, void * data)
{
	Clear();
	VkBufferCreateInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	Info.pNext = NULL;
	Info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	Info.size = count * stride;
	Info.queueFamilyIndexCount = 0;
	Info.pQueueFamilyIndices = NULL;
	Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	Info.flags = 0;

	V_CHK(vkCreateBuffer(Factory->Device, &Info, NULL, &Buffer));



	vkGetBufferMemoryRequirements(Factory->Device, Buffer, &MRequirements);

	VkMemoryAllocateInfo AllocateInfo = {};
	AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	AllocateInfo.pNext = NULL;
	AllocateInfo.memoryTypeIndex = 0;
	AllocateInfo.allocationSize = MRequirements.size;


	BEAR_ASSERT(Test(MRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &AllocateInfo.memoryTypeIndex))


	V_CHK(vkAllocateMemory(Factory->Device, &AllocateInfo, NULL, &Memory));
	
	VertexDescription.binding = 0;
	VertexDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VertexDescription.stride =static_cast<uint32_t>( stride);
	if (data)
	{
		bear_copy(Lock(), data, count*stride);
		Unlock();
	}
}

void * VKRenderVertexBuffer::Lock()
{
	if (Memory == 0)return 0;
	uint8_t *pData;
	V_CHK(vkMapMemory(Factory->Device, Memory, 0, MRequirements.size, 0, (void **)&pData));
	return pData;
}

void VKRenderVertexBuffer::Unlock()
{
	if (Memory == 0)return;
	vkUnmapMemory(Factory->Device, Memory);
	V_CHK(vkBindBufferMemory(Factory->Device, Buffer, Memory, 0));
}

void VKRenderVertexBuffer::Clear()
{
	if (Buffer)vkDestroyBuffer(Factory->Device, Buffer,0);
	Buffer = 0;
	if (Memory)vkFreeMemory(Factory->Device, Memory, 0);
	Memory = 0;
}