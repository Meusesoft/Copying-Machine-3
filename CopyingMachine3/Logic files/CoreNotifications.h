#pragma once

enum eCoreNotification {eNotificationNewDocument, 
						eNotificationDocumentViewChanges,
						eNotificationPrintSettingsChanged,
						eNotificationPrintDeviceChanged, 
						eNotificationScanSettingsChanged, 
						eNotificationScanDeviceChanged,
						eNotificationNewPage,
						eNotificationPageDeleted,
						eNotificationPageViewChanges,
						eNotificationPageUpdate,
						eNotificationPageSettingUpdate,
						eNotificationApplicationInitialized,
						eNotificationWindowsInitialized,
						eNotificationAcquisitionPageDone,
						eNotificationDocumentPrintingDone,
						eNotificationPageOCRDone,
						eNotificationLanguageChanged,
						eNotificationDocumentClosed,
						eNotificationPaneChange,
						eNotificationPaneSizeChange,
						eNotificationUpdateControls,
						eNotificationColorChanged,
						eNotificationDocumentUpdate,
						eNotificationDocumentCollectionUpdate,
						eNotificationDocumentCollectionSelection,
						eNotificationDocumentCollectionFolderSelect};

struct sCoreNotification {

	eCoreNotification 	eNotification;	//the notification from the core
	HWND 				hSubscriber;	//the handle of the window of the subscriber 
	void*				pData;		//it depends on the notification what pData is
};

struct sSubscription {

	eCoreNotification 	eNotification;	//the notification from the core
	HWND 				hSubscriber;	//the handle of the window of the subscriber 
};

class CCoreNotifications {


public:
	CCoreNotifications();
	~CCoreNotifications();

	//function for the core classes to communicate events
	bool CommunicateEvent(eCoreNotification peNotification, void* pData);

	//functions for info about notifications and for retrieving notifications
	int NotificationCount(HWND phWnd);
	int NotificationSpecificCount(HWND phWnd, eCoreNotification peNotification);
	bool GetNotification(HWND phWnd, sCoreNotification& pcNotification);
	bool GetNotificationSpecific(HWND phWnd, sCoreNotification& pcNotification, eCoreNotification peNotification);
	bool RemoveNotificationSpecific(HWND phWnd, eCoreNotification peNotification);

	//function for subscribing and unsubscribing to events
	void Subscribe(HWND phWnd, eCoreNotification peNotification);
	bool Unsubscribe(HWND phWnd, eCoreNotification peNotification);
	bool UnsubscribeAll(HWND phWnd);

private:

	vector <sCoreNotification> oNotifications;
	vector <sSubscription> oSubscriptions;
	};