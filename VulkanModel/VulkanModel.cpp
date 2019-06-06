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


static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack( VkDebugUtilsMessageSeverityFlagBitsEXT messageSverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callBackData,
	void* userData )
{
	std::cerr << "Message::Vulkan::ValidationLayer: " << callBackData->pMessage << "\n";

	return VK_FALSE;
}

struct Application
{
	Application( unsigned int width_, unsigned int height_, std::string title, unsigned int amountOfInstances );
	void Initialize( unsigned int width_, unsigned int height_, std::string title, unsigned int amountOfInstances );
	void InitializeGLFW( unsigned int width_, unsigned int height_, std::string title );
	void InitializeVulkan( std::string name, const VkInstance& instance );
	void CreateInstance( std::string name, const VkInstance& instance );
	void GrabPhysicalDevices( const VkInstance& toGrabFrom );
	size_t CreateLogicalDevice( const VkInstance& instance, const VkPhysicalDevice& physicalDevice );
	void PrintAvailibleExtensions( const VkInstance& instance );
	VkResult InitializeVulkanDebugLayer( std::vector< const char* >&& validationLayers, const VkInstance& instance );
	bool Update();
	bool GLFWUpdate();
	void Destroy();
	void DestroyInstance( const VkInstance& instance );
	protected:
		GLFWwindow* window;
		std::vector< VkInstance > instances;
		VkApplicationInfo applicationInfo = {};
		VkInstanceCreateInfo createInfo = {};
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
		const char** glfwExtensions;
		uint32_t glfwExtensionCount = 0;
		VkDebugUtilsMessengerEXT debugMessenger;
		PFN_vkCreateDebugUtilsMessengerEXT createFunction;
		PFN_vkDestroyDebugUtilsMessengerEXT destroyFunction;
		std::vector< VkPhysicalDevice > allPhysicalDevices;
		std::vector< std::pair< VkDevice, uint32_t > > logicalDevices;
		unsigned int width = 800;
		unsigned int height = 600;
		float queuePriority = 1.0f;
};

int main()
{
	Application application{ 800, 600, "Vulkan Example", 1 };
	while( application.Update() );
	application.Destroy();
	return 0;
}

Application::Application( unsigned int width_, unsigned int height_, std::string title, unsigned int amountOfInstances ) {
	Initialize( width_, height_, title, amountOfInstances );
}

void Application::Initialize( unsigned int width_, unsigned int height_, std::string title, unsigned int amountOfInstances )
{
	InitializeGLFW( width_, height_, title );
	instances.resize( amountOfInstances );
	for( unsigned int i = 0; i < amountOfInstances; ++i )
		InitializeVulkan( title, instances[ i ] );
}
void Application::InitializeGLFW( unsigned int width_, unsigned int height_, std::string title )
{
	width = width_;
	height = height_;
	glfwInit();
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
	window = glfwCreateWindow( width, height, title.c_str(), nullptr, nullptr );
}
void Application::PrintAvailibleExtensions( const VkInstance& instance )
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
	std::vector< VkExtensionProperties > extensions( extensionCount );
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions.data() );
	std::cout << "Available Extensions: \n";
	for(const auto& currentExtension : extensions)
		std::cout << "\t" << currentExtension.extensionName << "\n";
}
void Application::InitializeVulkan( std::string name, const VkInstance& instance )
{
	CreateInstance( name, instance );
	if( allPhysicalDevices.size() == 0 )
		GrabPhysicalDevices( instance );
	CreateLogicalDevice( instance, allPhysicalDevices[ allPhysicalDevices.size() - 1 ] );
	allPhysicalDevices.pop_back();
}
void Application::CreateInstance( std::string name, const VkInstance& instance )
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
	PrintAvailibleExtensions( instance );
	glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );
	std::vector< const char* > enabledExtensions( glfwExtensions, glfwExtensions + glfwExtensionCount );
	createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size() );
	createInfo.ppEnabledLayerNames = validationLayers.data();

	enabledExtensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
	enabledExtensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

	createInfo.enabledExtensionCount = enabledExtensions.size();
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();

	if(vkCreateInstance( &createInfo, nullptr, &( ( VkInstance& ) instance ) ) != VK_SUCCESS) {
		std::cerr << "Error::InitializeVulkan( std::string ):void: Failed to create instance.\n";
		return;
	}
	if( debug == true )
	{
		std::cout << "Note::InitializeVulkan( std::string ):void: Debug mode, enabling validation layers.\n";
		auto result = InitializeVulkanDebugLayer( std::move( validationLayers ), instance );
		if( result == VK_SUCCESS )
			std::cout << "Note::InitializeVulkan( std::string ):void: Debug layer creation succesful!\n";
	}
	if(glfwVulkanSupported())
		std::cout << "Note::InitializeVulkan( std::string ):void: GLFW Supports Vulkan!\n";
	else
		std::cout << "Error::InitializeVulkan( std::string ):void: GLFW Does Not Support Vulkan!\n";
}
void Application::GrabPhysicalDevices( const VkInstance& toGrabFrom )
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	uint32_t amountOfAvailiblePhysicalDevices;
	vkEnumeratePhysicalDevices( toGrabFrom, &amountOfAvailiblePhysicalDevices, nullptr );
	std::cout << "Note::GrabPhysicalDevice( const VkInstance& toGrabFrom ):void: Amount of Physical Devices Availible: " << amountOfAvailiblePhysicalDevices << ".\n";
	if( amountOfAvailiblePhysicalDevices == 0 ) {
		std::cerr << "Error::GrabPhysicalDevice( const VkInstance& toGrabFrom ):void: No availible physical devices.\n";
		return;
	}
	allPhysicalDevices.clear();
	allPhysicalDevices.resize( amountOfAvailiblePhysicalDevices );
	vkEnumeratePhysicalDevices( toGrabFrom, &amountOfAvailiblePhysicalDevices, allPhysicalDevices.data() );
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

size_t Application::CreateLogicalDevice( const VkInstance& instance, const VkPhysicalDevice& physicalDevice )
{
	uint32_t queueFamilyCount = 0;
	std::vector< VkQueueFamilyProperties > queueFamilies;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo logicalDeviceCreationInfo = {};
	VkDevice logicalDevice;
	unsigned int queueFamilyIndex = -1;
	logicalDeviceCreationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, nullptr );
	queueFamilies.resize( queueFamilyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, queueFamilies.data() );
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	const size_t AMOUNT_OF_QUEUE_FAMILIES_CONSTANT = queueFamilies.size();
	std::cout << "Note::CreateLogicalDevice( const VkInstance&, const VkPhysicalDevice& ):void: Amount of queue families is " << AMOUNT_OF_QUEUE_FAMILIES_CONSTANT << ".\n";
	for( unsigned int i = 0; i < AMOUNT_OF_QUEUE_FAMILIES_CONSTANT; ++i )
	{
		if(queueFamilies[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queueFamilyIndex = i;
			break;
		}
	}
	if( queueFamilyIndex == -1 )
		std::cerr << "Error::CreateLogicalDevice( const VkInstance&, const VkPhysicalDevice& ):void: Failed to find suitable queue family.\n";
	queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;
	logicalDeviceCreationInfo.pQueueCreateInfos = &queueCreateInfo;
	logicalDeviceCreationInfo.queueCreateInfoCount = 1;
	logicalDeviceCreationInfo.pEnabledFeatures = &deviceFeatures;
	if( vkCreateDevice( physicalDevice, &logicalDeviceCreationInfo, nullptr, &logicalDevice ) != VK_SUCCESS ) {
		std::cerr << "Error::CreateLogicalDevice( const VkInstance&, const VkPhysicalDevice& ):void: Failed to create logic device.\n";
		logicalDevice = VK_NULL_HANDLE;
	}
	logicalDevices.push_back( std::pair< VkDevice, int >{ logicalDevice, queueFamilyIndex } );
	return ( logicalDevices.size() - 1 );
}

VkResult Application::InitializeVulkanDebugLayer( std::vector< const char* >&& validationLayers, const VkInstance& instance )
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
				std::cout << "Note::InitializeVulkanDebugLayer():VkResult: Comfirmed layer " << layerProperties.layerName << ".\n";
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
bool Application::Update() {
	return GLFWUpdate();
}
bool Application::GLFWUpdate()
{
	if(glfwWindowShouldClose( window ) == false) {
		glfwPollEvents();
		return true;
	}
	return false;
}
void Application::DestroyInstance( const VkInstance& instance )
{
	if( debug == true )
		destroyFunction( instance, debugMessenger, nullptr );
	vkDestroyInstance( instance, nullptr );
}
void Application::Destroy()
{
	const size_t AMOUNT_OF_LOGICAL_DEVICES_CONSTANT = logicalDevices.size();
	for( unsigned int i = 0; i < AMOUNT_OF_LOGICAL_DEVICES_CONSTANT; ++i )
		vkDestroyDevice( logicalDevices[ i ].first, nullptr );
	const size_t AMOUNT_OF_INSTANCES_CONSTANT = instances.size();
	for( unsigned int i = 0; i < AMOUNT_OF_INSTANCES_CONSTANT; ++i )
		DestroyInstance( instances[ i ] );
	glfwDestroyWindow( window );
	glfwTerminate();
}
