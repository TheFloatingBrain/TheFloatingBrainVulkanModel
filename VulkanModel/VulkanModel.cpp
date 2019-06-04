#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define NDEBUG

#ifdef NDEBUG
	bool debug = true;
#else
	bool debug = false;
#endif

unsigned int width = 800;
unsigned int height = 600;

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack( VkDebugUtilsMessageSeverityFlagBitsEXT messageSverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callBackData,
	void* userData )
{
	std::cerr << "Error::Vulkan::ValidationLayer: " << callBackData->pMessage << "\n";

	return VK_FALSE;
}

void Initialize( unsigned int width_, unsigned int height_, std::string title );
void InitializeGLFW( unsigned int width_, unsigned int height_, std::string title );
void InitializeVulkan( std::string name );
VkResult InitializeVulkanDebugLayer();
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

const char** glfwExtensions;
uint32_t glfwExtensionCount = 0;


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

	createInfo.ppEnabledExtensionNames = glfwExtensions;

	if( debug == true )
	{
		std::cout << "Debug mode, enabling validation layers.\n";
		InitializeVulkanDebugLayer();
	}

	if( vkCreateInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS ) {
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
VkResult InitializeVulkanDebugLayer()
{


	glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

	std::vector< const char* > enabledExtensions( glfwExtensions, glfwExtensions + glfwExtensionCount );

	VkDebugUtilsMessengerEXT debugMessenger;
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};

	debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessengerCreateInfo.pfnUserCallback = DebugCallBack;
	debugMessengerCreateInfo.pUserData = nullptr;

	std::vector< const char* > validationLayers{ "VK_LAYER_KHRONOS_validation" };

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

	std::vector< VkLayerProperties > availableLayers( layerCount );
	vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data() );

	createInfo.enabledLayerCount = 1;
	createInfo.ppEnabledLayerNames = validationLayers.data();

	enabledExtensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

	createInfo.enabledExtensionCount = enabledExtensions.size();

	createInfo.ppEnabledExtensionNames = enabledExtensions.data();


	auto function = ( PFN_vkCreateDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );


	if(function != nullptr) {
		return function( instance, &debugMessengerCreateInfo, nullptr, &debugMessenger );
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
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
