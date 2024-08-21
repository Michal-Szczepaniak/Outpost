#ifndef PARCELLIST_H
#define PARCELLIST_H

#include <QAbstractListModel>
#include <QObject>
#include "apiclient.h"

class ParcelList : public QAbstractListModel
{
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_OBJECT
public:
    enum ParcelRoles {
        IdRole = Qt::UserRole + 1,
        ShipmentNumberRole,
        SenderNameRole,
        OpenCodeRole,
        SizeRole,
        QrCodeRole,
        StatusRole,
        TypeRole,
        OwnershipRole
    };

    enum class ParcelStatus {
        CREATED,
        ADOPTED_AT_SORTING_CENTER,
        ADOPTED_AT_SOURCE_BRANCH,
        ADOPTED_AT_TARGET_BRANCH,
        AVIZO,
        CANCELED,
        CANCELED_REDIRECT_TO_BOX,
        CLAIMED,
        COLLECTED_FROM_SENDER,
        CONFIRMED,
        DELAY_IN_DELIVERY,
        DELIVERED,
        DISPATCHED_BY_SENDER,
        DISPATCHED_BY_SENDER_TO_POK,
        MISSING,
        OFFER_SELECTED,
        OFFERS_PREPARED,
        OTHER,
        OUT_FOR_DELIVERY,
        OUT_FOR_DELIVERY_TO_ADDRESS,
        OVERSIZED,
        PICKUP_REMINDER_SENT_ADDRESS,
        PICKUP_REMINDER_SENT,
        PICKUP_TIME_EXPIRED,
        READDRESSED,
        READY_TO_PICKUP,
        READY_TO_PICKUP_FROM_BRANCH,
        READY_TO_PICKUP_FROM_POK,
        READY_TO_PICKUP_FROM_POK_REGISTERED,
        REDIRECT_TO_BOX,
        REJECTED_BY_RECEIVER,
        RETURNED_TO_SENDER,
        RETURN_PICKUP_CONFIRMATION_TO_SENDER,
        SENT_FROM_SORTING_CENTER,
        SENT_FROM_SOURCE_BRANCH,
        STACK_IN_BOX_MACHINE,
        STACK_IN_CUSTOMER_SERVICE_POINT,
        STACK_PARCEL_PICKUP_TIME_EXPIRED,
        STACK_PARCEL_IN_BOX_MACHINE_PICKUP_TIME_EXPIRED,
        TAKEN_BY_COURIER,
        TAKEN_BY_COURIER_FROM_POK,
        UNDELIVERED,
        UNDELIVERED_COD_CASH_RECEIVER,
        UNDELIVERED_INCOMPLETE_ADDRESS,
        UNDELIVERED_LACK_OF_ACCESS_LETTERBOX,
        UNDELIVERED_NO_MAILBOX,
        UNDELIVERED_NOT_LIVE_ADDRESS,
        UNDELIVERED_UNKNOWN_RECEIVER,
        UNDELIVERED_WRONG_ADDRESS,
        UNSTACK_FROM_BOX_MACHINE,
        UNSTACK_FROM_CUSTOMER_SERVICE_POINT,
        COD_REJECTED,
        COD_COMPLETED,
        C2X_REJECTED,
        C2X_COMPLETED,
        AVIZO_REJECTED,
        AVIZO_COMPLETED,
    };
    Q_ENUM(ParcelStatus)

    enum class ParcelSize {
        A,
        B,
        C,
        OTHER
    };
    Q_ENUM(ParcelSize)

    enum class ParcelOwnershipStatus {
        OWN,
        FRIEND,
        OBSERVED,
        NOT_SUPPORTED
    };
    Q_ENUM(ParcelOwnershipStatus)

    enum class ParcelType {
        PARCEL,
        COURIER,
        MULTICOMPARTMENT,
        OTHER
    };
    Q_ENUM(ParcelType)

    struct Parcel {
        QString shipmentNumber;
        QString senderName;
        QString openCode;
        ParcelOwnershipStatus ownershipStatus;
        ParcelSize size;
        QString qrCode;
        ParcelStatus status = ParcelStatus::OTHER;
        ParcelType type = ParcelType::OTHER;
        QString multiCompartment = "";
    };

    explicit ParcelList(ApiClient *apiClient = nullptr, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = IdRole) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void load(unsigned int listTypeIndex);
    Q_INVOKABLE void load(ApiClient::ParcelListType listType = ApiClient::ParcelListType::Pending);

private:
    ParcelStatus parseParcelStatus(std::string parcelStatus);
    ParcelSize parseParcelSize(std::string parcelSize);
    ParcelOwnershipStatus parseOwnershipStatus(std::string ownershipStatus);
    ParcelType parseParcelType(std::string parcelType);
    QString translateParcelStatus(ParcelStatus status) const;
    QString translateParcelSize(ParcelSize size) const;
    QString translateParcelType(ParcelType type) const;

private:
    QVector<Parcel> _parcels;
    ApiClient *_apiClient;
    ApiClient::ParcelListType _listType;
    const QVector<ParcelStatus> _pendingStatuses = {
        ParcelStatus::READY_TO_PICKUP, ParcelStatus::CONFIRMED,
        ParcelStatus::ADOPTED_AT_SORTING_CENTER, ParcelStatus::ADOPTED_AT_SOURCE_BRANCH,
        ParcelStatus::COLLECTED_FROM_SENDER, ParcelStatus::DISPATCHED_BY_SENDER,
        ParcelStatus::DISPATCHED_BY_SENDER_TO_POK, ParcelStatus::OUT_FOR_DELIVERY,
        ParcelStatus::OUT_FOR_DELIVERY_TO_ADDRESS, ParcelStatus::SENT_FROM_SOURCE_BRANCH,
        ParcelStatus::TAKEN_BY_COURIER, ParcelStatus::TAKEN_BY_COURIER_FROM_POK,
        ParcelStatus::STACK_IN_BOX_MACHINE, ParcelStatus::STACK_IN_CUSTOMER_SERVICE_POINT
    };
};
Q_DECLARE_METATYPE(ParcelList::ParcelSize)

#endif // PARCELLIST_H
