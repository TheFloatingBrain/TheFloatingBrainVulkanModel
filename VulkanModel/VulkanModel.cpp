#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


unsigned int width = 800;
unsigned int height = 600;

void Initialize( unsigned int width_, unsigned int height_, std::string title );
void InitializeGLFW( unsigned int width_, unsigned int height_, std::string title );
void InitializeVulkan( std::string name );
bool Update();
bool GLFWUpdate();
void Destroy();

int main()
{
	Initialize( 800, 600, "Vulkan Example" );
	while( Update() );
	Destroy();
	return 0;
}

GLFWwindow* window;
VkInstance instance;
VkApplicationInfo applicationInfo = {};
VkInstanceCreateInfo createInfo = {};

void Initialize( unsigned int width_, unsigned int height_, std::string title ) {
	InitializeGLFW( width_, height_, title );
	InitializeVulkan( title );
}
void InitializeGLFW( unsigned int width_, unsigned int height_, std::string title )
{
	width = width_;
	height = height_;
	glfwInit();
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
	window = glfwCreateWindow( width, height, title.c_str(), nullptr, nullptr );
}
void InitializeVulkan( std::string name )
{
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = name.c_str();
	applicationInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	applicationInfo.pEngineName = "No Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	createInfo.enabledLayerCount = 0;

	if(vkCreateInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS) {
		std::cerr << "Error::InitializeVulkan( std::string ): Failed to create instance.\n";
		return;
	}

	uint32_t extensionCount = 0;

	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

	std::vector< VkExtensionProperties > extensions( extensionCount );

	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions.data() );

	std::cout << "Available Extensions: \n";

	for(const auto& currentExtension : extensions)
		std::cout << "\t" << currentExtension.extensionName << "\n";

	if(glfwVulkanSupported())
		std::cout << "GLFW Supports Vulkan\n";
	else
		std::cout << "GLFW Does Not Support Vulkan\n";
}
bool Update() {
	return GLFWUpdate();
}
bool GLFWUpdate()
{
	if(glfwWindowShouldClose( window ) == false) {
		glfwPollEvents();
		return true;
	}
	return false;
}
void Destroy() {
	vkDestroyInstance( instance, nullptr );
	glfwDestroyWindow( window );
	glfwTerminate();
}
