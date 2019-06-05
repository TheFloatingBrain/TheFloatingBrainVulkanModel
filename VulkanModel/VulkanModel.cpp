#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


const unsigned short NVDIA_VENDOR_ID_CONSTANT = 4318;

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
	std::cerr << "Message::Vulkan::ValidationLayer: " << callBackData->pMessage << "\n";

	return VK_FALSE;
}

void Initialize( unsigned int width_, unsigned int height_, std::string title );
void InitializeGLFW( unsigned int width_, unsigned int height_, std::string title );
void InitializeVulkan( std::string name );
void CreateInstance( std::string name );
void GrabPhysicalDevices( const VkInstance& toGrabFrom );
void PrintAvailibleExtensions();
VkResult InitializeVulkanDebugLayer( std::vector< const char* >&& validationLayers );
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

VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
const char** glfwExtensions;
uint32_t glfwExtensionCount = 0;
VkDebugUtilsMessengerEXT debugMessenger;
PFN_vkCreateDebugUtilsMessengerEXT createFunction;
PFN_vkDestroyDebugUtilsMessengerEXT destroyFunction;
std::vector< VkPhysicalDevice > allPhysicalDevices;


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
void PrintAvailibleExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
	std::vector< VkExtensionProperties > extensions( extensionCount );
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions.data() );
	std::cout << "Available Extensions: \n";
	for(const auto& currentExtension : extensions)
		std::cout << "\t" << currentExtension.extensionName << "\n";
}
void InitializeVulkan( std::string name ) {
	CreateInstance( name );
	GrabPhysicalDevices( instance );
}
void CreateInstance( std::string name )
{
	std::vector< const char* > validationLayers{ "VK_LAYER_KHRONOS_validation" };
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = name.c_str();
	applicationInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	applicationInfo.pEngineName = "No Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	PrintAvailibleExtensions();
	glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );
	std::vector< const char* > enabledExtensions( glfwExtensions, glfwExtensions + glfwExtensionCount );
	createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size() );
	createInfo.ppEnabledLayerNames = validationLayers.data();

	enabledExtensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
	enabledExtensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

	createInfo.enabledExtensionCount = enabledExtensions.size();
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();

	if(vkCreateInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS) {
		std::cerr << "Error::InitializeVulkan( std::string ):void: Failed to create instance.\n";
		return;
	}
	if( debug == true )
	{
		std::cout << "Note::InitializeVulkan( std::string ):void: Debug mode, enabling validation layers.\n";
		auto result = InitializeVulkanDebugLayer( std::move( validationLayers ) );
		if( result == VK_SUCCESS )
			std::cout << "Note::InitializeVulkan( std::string ):void: Debug layer creation succesful!\n";
	}
	if(glfwVulkanSupported())
		std::cout << "Note::InitializeVulkan( std::string ):void: GLFW Supports Vulkan\n";
	else
		std::cout << "Error::InitializeVulkan( std::string ):void: GLFW Does Not Support Vulkan\n";
}
void GrabPhysicalDevices( const VkInstance& toGrabFrom )
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	uint32_t amountOfAvailiblePhysicalDevices;
	vkEnumeratePhysicalDevices( instance, &amountOfAvailiblePhysicalDevices, nullptr );
	std::cout << "Note::GrabPhysicalDevice( const VkInstance& toGrabFrom ):void: Amount of Physical Devices Availible: " << amountOfAvailiblePhysicalDevices << ".\n";
	if( amountOfAvailiblePhysicalDevices == 0 ) {
		std::cerr << "Error::GrabPhysicalDevice( const VkInstance& toGrabFrom ):void: No availible physical devices.\n";
		return;
	}
	allPhysicalDevices.clear();
	allPhysicalDevices.resize( amountOfAvailiblePhysicalDevices );
	vkEnumeratePhysicalDevices( instance, &amountOfAvailiblePhysicalDevices, allPhysicalDevices.data() );
	std::cout << "Note::GrabPhysicalDevice( const VkInstance& toGrabFrom ):void: Found devices:\n";
	int nvidia = -1;
	const unsigned int AMOUNT_OF_DEVICES_CONSTANT = allPhysicalDevices.size();
	for( unsigned int i = 0; i < AMOUNT_OF_DEVICES_CONSTANT; ++i )
	{
		vkGetPhysicalDeviceProperties( allPhysicalDevices[ i ], &deviceProperties );
		vkGetPhysicalDeviceFeatures( allPhysicalDevices[ i ], &deviceFeatures );
		std::cout << "\t* : Device Name: " << deviceProperties.deviceName << "\n";
		if( deviceProperties.vendorID == NVDIA_VENDOR_ID_CONSTANT ) {
			std::cout << "\t\t--> This is an NVDIA device\n";
			nvidia = i;
		}
	}
	if( nvidia != ( -1 ) ) 
	{
		VkPhysicalDevice toSwap = allPhysicalDevices[ AMOUNT_OF_DEVICES_CONSTANT - 1 ];
		allPhysicalDevices[ AMOUNT_OF_DEVICES_CONSTANT - 1 ] = allPhysicalDevices[ nvidia ];
		allPhysicalDevices[ nvidia ] = toSwap;
	}
}
VkResult InitializeVulkanDebugLayer( std::vector< const char* >&& validationLayers )
{
	debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessengerCreateInfo.pfnUserCallback = DebugCallBack;
	debugMessengerCreateInfo.pUserData = nullptr;

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
	std::vector< VkLayerProperties > availableLayers( layerCount );
	vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data() );
	std::cout << "Note::InitializeVulkanDebugLayer():VkResult: Availible layer count is " << layerCount << ".\n";

	for( const char* layerName : validationLayers )
	{
		bool layerFound = false;
		for( const auto& layerProperties : availableLayers )
		{
			if( strcmp( layerName, layerProperties.layerName ) == 0 )
			{
				layerFound = true;
				std::cout << "Note::InitializeVulkanDebugLayer():VkResult: Comfirmed layer " << layerProperties.layerName << "\n";
				break;
			}
		}

		if( !layerFound ) {
			std::cerr << "Error::InitializeVulkanDebugLayer():VkResult: Failed to confirm all layer names.\n";
			return VkResult::VK_INCOMPLETE;
		}
	}


	createFunction = ( PFN_vkCreateDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );

	destroyFunction = ( PFN_vkDestroyDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );

	if( createFunction != nullptr )
		return createFunction( instance, &debugMessengerCreateInfo, nullptr, &debugMessenger );
	else {
		std::cerr << "Error::InitializeVulkanDebugLayer():VkResult: vkCreateDebugUtilsMessengerEXT function is null.\n";
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
void Destroy()
{
	if(debug == true)
		destroyFunction( instance, debugMessenger, nullptr );
	vkDestroyInstance( instance, nullptr );
	glfwDestroyWindow( window );
	glfwTerminate();
}
