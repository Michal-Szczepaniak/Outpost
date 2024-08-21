#include "parcellist.h"
#include <QDebug>

ParcelList::ParcelList(ApiClient *apiClient, QObject *parent) : QAbstractListModel(parent)
{
    _apiClient = apiClient;
}

int ParcelList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _parcels.size();
}

QVariant ParcelList::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() >= rowCount()) return QVariant();

    const Parcel& parcel = _parcels[index.row()];

    switch (role) {
    case ParcelRoles::IdRole:
        return index.row();
    case ParcelRoles::ShipmentNumberRole:
        return parcel.shipmentNumber;
    case ParcelRoles::SenderNameRole:
        return parcel.senderName;
    case ParcelRoles::OpenCodeRole:
        return parcel.openCode;
    case ParcelRoles::SizeRole:
        return translateParcelSize(parcel.size);
    case ParcelRoles::QrCodeRole:
        return parcel.qrCode;
    case ParcelRoles::StatusRole:
        return translateParcelStatus(parcel.status);
    case ParcelRoles::TypeRole:
        return translateParcelType(parcel.type);
    case ParcelRoles::OwnershipRole:
        return static_cast<unsigned int>(parcel.ownershipStatus);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ParcelList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[ShipmentNumberRole] = "shipmentNumber";
    roles[SenderNameRole] = "senderName";
    roles[OpenCodeRole] = "openCode";
    roles[SizeRole] = "parcelSize";
    roles[QrCodeRole] = "qrCode";
    roles[StatusRole] = "parcelStatus";
    roles[TypeRole] = "parcelType";
    roles[OwnershipRole] = "parcelOwnership";
    return roles;
}

void ParcelList::load(unsigned int listTypeIndex)
{
    if (listTypeIndex > 3) return;

    load(static_cast<ApiClient::ParcelListType>(listTypeIndex));
}

void ParcelList::load(ApiClient::ParcelListType listType)
{
    _listType = listType;

    nlohmann::json data = _apiClient->getParcels(_listType);
    auto parcels = data["parcels"];

    beginResetModel();
    _parcels.clear();
    for (auto parcel : parcels) {
        Parcel newParcel{
            .shipmentNumber = QString::fromStdString(parcel["shipmentNumber"]),
            .openCode = QString::fromStdString(parcel.value("openCode", "")),
            .ownershipStatus = parseOwnershipStatus(parcel.value("ownershipStatus", "")),
            .size = parseParcelSize(parcel.value("parcelSize", "")),
            .qrCode = QString::fromStdString(parcel.value("qrCode", "")),
            .status = parseParcelStatus(parcel.value("status", "")),
            .type = parseParcelType(parcel.value("shipmentType", ""))
        };

        if (_listType == ApiClient::ParcelListType::Sent) {
            if (parcel.contains("receiver")) {
                newParcel.senderName = QString::fromStdString(parcel["receiver"].value("name", ""));
            }
        } else {
            if (parcel.contains("sender")) {
                newParcel.senderName = QString::fromStdString(parcel["sender"].value("name", ""));
            }
        }

        if (_listType == ApiClient::Pending && !_pendingStatuses.contains(newParcel.status)) continue;

        if (parcel.contains("multiCompartment")) {
            if (parcel["multiCompartment"].contains("shipmentNumbers")) {
                QString shipmentNumbers;

                for (nlohmann::json number : parcel["multiCompartment"]["shipmentNumbers"]) {
                    shipmentNumbers += QString::fromStdString(number) + ",";
                }

                shipmentNumbers = shipmentNumbers.left(shipmentNumbers.length()-1);

                newParcel.multiCompartment = shipmentNumbers;
                newParcel.type = ParcelType::MULTICOMPARTMENT;
            } else {
                continue;
            }
        }
        _parcels.push_front(newParcel);
    }

    endResetModel();
}

ParcelList::ParcelStatus ParcelList::parseParcelStatus(std::string parcelStatus)
{
    if (parcelStatus == "CREATED")
        return ParcelStatus::CREATED;
    if (parcelStatus == "ADOPTED_AT_SORTING_CENTER")
        return ParcelStatus::ADOPTED_AT_SORTING_CENTER;
    if (parcelStatus == "ADOPTED_AT_SOURCE_BRANCH")
        return ParcelStatus::ADOPTED_AT_SOURCE_BRANCH;
    if (parcelStatus == "ADOPTED_AT_TARGET_BRANCH")
        return ParcelStatus::ADOPTED_AT_TARGET_BRANCH;
    if (parcelStatus == "AVIZO")
        return ParcelStatus::AVIZO;
    if (parcelStatus == "CANCELED")
        return ParcelStatus::CANCELED;
    if (parcelStatus == "CANCELED_REDIRECT_TO_BOX")
        return ParcelStatus::CANCELED_REDIRECT_TO_BOX;
    if (parcelStatus == "CLAIMED")
        return ParcelStatus::CLAIMED;
    if (parcelStatus == "COLLECTED_FROM_SENDER")
        return ParcelStatus::COLLECTED_FROM_SENDER;
    if (parcelStatus == "CONFIRMED")
        return ParcelStatus::CONFIRMED;
    if (parcelStatus == "DELAY_IN_DELIVERY")
        return ParcelStatus::DELAY_IN_DELIVERY;
    if (parcelStatus == "DELIVERED")
        return ParcelStatus::DELIVERED;
    if (parcelStatus == "DISPATCHED_BY_SENDER")
        return ParcelStatus::DISPATCHED_BY_SENDER;
    if (parcelStatus == "DISPATCHED_BY_SENDER_TO_POK")
        return ParcelStatus::DISPATCHED_BY_SENDER_TO_POK;
    if (parcelStatus == "MISSING")
        return ParcelStatus::MISSING;
    if (parcelStatus == "OFFER_SELECTED")
        return ParcelStatus::OFFER_SELECTED;
    if (parcelStatus == "OFFERS_PREPARED")
        return ParcelStatus::OFFERS_PREPARED;
    if (parcelStatus == "OUT_FOR_DELIVERY")
        return ParcelStatus::OUT_FOR_DELIVERY;
    if (parcelStatus == "OUT_FOR_DELIVERY_TO_ADDRESS")
        return ParcelStatus::OUT_FOR_DELIVERY_TO_ADDRESS;
    if (parcelStatus == "OVERSIZED")
        return ParcelStatus::OVERSIZED;
    if (parcelStatus == "PICKUP_REMINDER_SENT_ADDRESS")
        return ParcelStatus::PICKUP_REMINDER_SENT_ADDRESS;
    if (parcelStatus == "PICKUP_REMINDER_SENT")
        return ParcelStatus::PICKUP_REMINDER_SENT;
    if (parcelStatus == "PICKUP_TIME_EXPIRED")
        return ParcelStatus::PICKUP_TIME_EXPIRED;
    if (parcelStatus == "READDRESSED")
        return ParcelStatus::READDRESSED;
    if (parcelStatus == "READY_TO_PICKUP")
        return ParcelStatus::READY_TO_PICKUP;
    if (parcelStatus == "READY_TO_PICKUP_FROM_BRANCH")
        return ParcelStatus::READY_TO_PICKUP_FROM_BRANCH;
    if (parcelStatus == "READY_TO_PICKUP_FROM_POK")
        return ParcelStatus::READY_TO_PICKUP_FROM_POK;
    if (parcelStatus == "READY_TO_PICKUP_FROM_POK_REGISTERED")
        return ParcelStatus::READY_TO_PICKUP_FROM_POK_REGISTERED;
    if (parcelStatus == "REDIRECT_TO_BOX")
        return ParcelStatus::REDIRECT_TO_BOX;
    if (parcelStatus == "REJECTED_BY_RECEIVER")
        return ParcelStatus::REJECTED_BY_RECEIVER;
    if (parcelStatus == "RETURNED_TO_SENDER")
        return ParcelStatus::RETURNED_TO_SENDER;
    if (parcelStatus == "RETURN_PICKUP_CONFIRMATION_TO_SENDER")
        return ParcelStatus::RETURN_PICKUP_CONFIRMATION_TO_SENDER;
    if (parcelStatus == "SENT_FROM_SORTING_CENTER")
        return ParcelStatus::SENT_FROM_SORTING_CENTER;
    if (parcelStatus == "SENT_FROM_SOURCE_BRANCH")
        return ParcelStatus::SENT_FROM_SOURCE_BRANCH;
    if (parcelStatus == "STACK_IN_BOX_MACHINE")
        return ParcelStatus::STACK_IN_BOX_MACHINE;
    if (parcelStatus == "STACK_IN_CUSTOMER_SERVICE_POINT")
        return ParcelStatus::STACK_IN_CUSTOMER_SERVICE_POINT;
    if (parcelStatus == "STACK_PARCEL_PICKUP_TIME_EXPIRED")
        return ParcelStatus::STACK_PARCEL_PICKUP_TIME_EXPIRED;
    if (parcelStatus == "STACK_PARCEL_IN_BOX_MACHINE_PICKUP_TIME_EXPIRED")
        return ParcelStatus::STACK_PARCEL_IN_BOX_MACHINE_PICKUP_TIME_EXPIRED;
    if (parcelStatus == "TAKEN_BY_COURIER")
        return ParcelStatus::TAKEN_BY_COURIER;
    if (parcelStatus == "TAKEN_BY_COURIER_FROM_POK")
        return ParcelStatus::TAKEN_BY_COURIER_FROM_POK;
    if (parcelStatus == "UNDELIVERED")
        return ParcelStatus::UNDELIVERED;
    if (parcelStatus == "UNDELIVERED_COD_CASH_RECEIVER")
        return ParcelStatus::UNDELIVERED_COD_CASH_RECEIVER;
    if (parcelStatus == "UNDELIVERED_INCOMPLETE_ADDRESS")
        return ParcelStatus::UNDELIVERED_INCOMPLETE_ADDRESS;
    if (parcelStatus == "UNDELIVERED_LACK_OF_ACCESS_LETTERBOX")
        return ParcelStatus::UNDELIVERED_LACK_OF_ACCESS_LETTERBOX;
    if (parcelStatus == "UNDELIVERED_NO_MAILBOX")
        return ParcelStatus::UNDELIVERED_NO_MAILBOX;
    if (parcelStatus == "UNDELIVERED_NOT_LIVE_ADDRESS")
        return ParcelStatus::UNDELIVERED_NOT_LIVE_ADDRESS;
    if (parcelStatus == "UNDELIVERED_UNKNOWN_RECEIVER")
        return ParcelStatus::UNDELIVERED_UNKNOWN_RECEIVER;
    if (parcelStatus == "UNDELIVERED_WRONG_ADDRESS")
        return ParcelStatus::UNDELIVERED_WRONG_ADDRESS;
    if (parcelStatus == "UNSTACK_FROM_BOX_MACHINE")
        return ParcelStatus::UNSTACK_FROM_BOX_MACHINE;
    if (parcelStatus == "UNSTACK_FROM_CUSTOMER_SERVICE_POINT")
        return ParcelStatus::UNSTACK_FROM_CUSTOMER_SERVICE_POINT;
    if (parcelStatus == "COD_REJECTED")
        return ParcelStatus::COD_REJECTED;
    if (parcelStatus == "COD_COMPLETED")
        return ParcelStatus::COD_COMPLETED;
    if (parcelStatus == "C2X_REJECTED")
        return ParcelStatus::C2X_REJECTED;
    if (parcelStatus == "C2X_COMPLETED")
        return ParcelStatus::C2X_COMPLETED;
    if (parcelStatus == "AVIZO_REJECTED")
        return ParcelStatus::AVIZO_REJECTED;
    if (parcelStatus == "AVIZO_COMPLETED")
        return ParcelStatus::AVIZO_COMPLETED;
    return ParcelStatus::OTHER;
}

ParcelList::ParcelSize ParcelList::parseParcelSize(std::string parcelSize)
{
    if (parcelSize == "A")
        return ParcelSize::A;
    if (parcelSize == "B")
        return ParcelSize::B;
    if (parcelSize == "C")
        return ParcelSize::C;
    return ParcelSize::OTHER;
}

ParcelList::ParcelOwnershipStatus ParcelList::parseOwnershipStatus(std::string ownershipStatus)
{
    if (ownershipStatus == "OWN")
        return ParcelOwnershipStatus::OWN;
    if (ownershipStatus == "FRIEND")
        return ParcelOwnershipStatus::FRIEND;
    if (ownershipStatus == "OBSERVED")
        return ParcelOwnershipStatus::OBSERVED;
    return ParcelOwnershipStatus::NOT_SUPPORTED;
}

ParcelList::ParcelType ParcelList::parseParcelType(std::string parcelType)
{
    if (parcelType == "parcel")
        return ParcelType::PARCEL;
    if (parcelType == "courier")
        return ParcelType::COURIER;
    return ParcelType::OTHER;
}

QString ParcelList::translateParcelStatus(ParcelStatus status) const
{
    switch (status) {
    case ParcelStatus::CREATED:
        return tr("Created");
    case ParcelStatus::ADOPTED_AT_SORTING_CENTER:
        return tr("Adopted at sorting center");
    case ParcelStatus::ADOPTED_AT_SOURCE_BRANCH:
        return tr("Adopted at source branch");
    case ParcelStatus::ADOPTED_AT_TARGET_BRANCH:
        return tr("Adopted at target branch");
    case ParcelStatus::AVIZO:
        return tr("Avizo");
    case ParcelStatus::CANCELED:
        return tr("Cancelled");
    case ParcelStatus::CANCELED_REDIRECT_TO_BOX:
        return tr("Canceled redirect to box");
    case ParcelStatus::CLAIMED:
        return tr("Claimed");
    case ParcelStatus::COLLECTED_FROM_SENDER:
        return tr("Collected from sender");
    case ParcelStatus::CONFIRMED:
        return tr("Confirmed");
    case ParcelStatus::DELAY_IN_DELIVERY:
        return tr("Delay in delivery");
    case ParcelStatus::DELIVERED:
        return tr("Delivered");
    case ParcelStatus::DISPATCHED_BY_SENDER:
        return tr("Dispatched by sender");
    case ParcelStatus::DISPATCHED_BY_SENDER_TO_POK:
        return tr("Dispatched by sender to POK");
    case ParcelStatus::MISSING:
        return tr("Missing");
    case ParcelStatus::OFFER_SELECTED:
        return tr("Offer selected");
    case ParcelStatus::OFFERS_PREPARED:
        return tr("Offers prepared");
    case ParcelStatus::OUT_FOR_DELIVERY:
        return tr("Out for delivery");
    case ParcelStatus::OUT_FOR_DELIVERY_TO_ADDRESS:
        return tr("Out for delivery to address");
    case ParcelStatus::OVERSIZED:
        return tr("Oversized");
    case ParcelStatus::PICKUP_REMINDER_SENT_ADDRESS:
        return tr("Pickup reminder sent address");
    case ParcelStatus::PICKUP_REMINDER_SENT:
        return tr("Pickup reminder sent");
    case ParcelStatus::PICKUP_TIME_EXPIRED:
        return tr("Pickup time expired");
    case ParcelStatus::READDRESSED:
        return tr("Re-addressed");
    case ParcelStatus::READY_TO_PICKUP:
        return tr("Ready to pickup");
    case ParcelStatus::READY_TO_PICKUP_FROM_BRANCH:
        return tr("Ready to pickup from branch");
    case ParcelStatus::READY_TO_PICKUP_FROM_POK:
        return tr("Ready to pickup from POK");
    case ParcelStatus::READY_TO_PICKUP_FROM_POK_REGISTERED:
        return tr("Ready to pickup from POK registered");
    case ParcelStatus::REDIRECT_TO_BOX:
        return tr("Redirect to box");
    case ParcelStatus::REJECTED_BY_RECEIVER:
        return tr("Rejected by receiver");
    case ParcelStatus::RETURNED_TO_SENDER:
        return tr("Returned to sender");
    case ParcelStatus::RETURN_PICKUP_CONFIRMATION_TO_SENDER:
        return tr("Return pickup confirmation to sender");
    case ParcelStatus::SENT_FROM_SORTING_CENTER:
        return tr("Sent from sorting center");
    case ParcelStatus::SENT_FROM_SOURCE_BRANCH:
        return tr("Sent from source branch");
    case ParcelStatus::STACK_IN_BOX_MACHINE:
        return tr("Stack in box machine");
    case ParcelStatus::STACK_IN_CUSTOMER_SERVICE_POINT:
        return tr("Stack in customer service point");
    case ParcelStatus::STACK_PARCEL_PICKUP_TIME_EXPIRED:
        return tr("Stack parcel pickup time expired");
    case ParcelStatus::STACK_PARCEL_IN_BOX_MACHINE_PICKUP_TIME_EXPIRED:
        return tr("Stack parcel in box machine pickup time expired");
    case ParcelStatus::TAKEN_BY_COURIER:
        return tr("Taken by courier");
    case ParcelStatus::TAKEN_BY_COURIER_FROM_POK:
        return tr("Taken by courier from POK");
    case ParcelStatus::UNDELIVERED:
        return tr("Undelivered");
    case ParcelStatus::UNDELIVERED_COD_CASH_RECEIVER:
        return tr("Undelivered cod cash receiver");
    case ParcelStatus::UNDELIVERED_INCOMPLETE_ADDRESS:
        return tr("Undelivered incomplete address");
    case ParcelStatus::UNDELIVERED_LACK_OF_ACCESS_LETTERBOX:
        return tr("Undelivered lack of access letterbox");
    case ParcelStatus::UNDELIVERED_NO_MAILBOX:
        return tr("Undelivered no mailbox");
    case ParcelStatus::UNDELIVERED_NOT_LIVE_ADDRESS:
        return tr("Undelivered not live address");
    case ParcelStatus::UNDELIVERED_UNKNOWN_RECEIVER:
        return tr("Undelivered unknown receiver");
    case ParcelStatus::UNDELIVERED_WRONG_ADDRESS:
        return tr("Undelivered wrong address");
    case ParcelStatus::UNSTACK_FROM_BOX_MACHINE:
        return tr("Unstack from box macine");
    case ParcelStatus::UNSTACK_FROM_CUSTOMER_SERVICE_POINT:
        return tr("Unstack from customer service point");
    case ParcelStatus::COD_REJECTED:
        return tr("COD rejected");
    case ParcelStatus::COD_COMPLETED:
        return tr("COD completed");
    case ParcelStatus::C2X_REJECTED:
        return tr("C2X rejected");
    case ParcelStatus::C2X_COMPLETED:
        return tr("C2X completed");
    case ParcelStatus::AVIZO_REJECTED:
        return tr("Avizo rejected");
    case ParcelStatus::AVIZO_COMPLETED:
        return tr("Avizo completed");
    default:
        return tr("Other");
    }
}

QString ParcelList::translateParcelSize(ParcelSize size) const
{
    switch (size) {
    case ParcelSize::A:
        return tr("a");
    case ParcelSize::B:
        return tr("b");
    case ParcelSize::C:
        return tr("c");
    default:
        return tr("o");
    }
}

QString ParcelList::translateParcelType(ParcelType type) const
{
    switch (type) {
    case ParcelType::PARCEL:
        return tr("Parcel");
    case ParcelType::COURIER:
        return tr("Courier");
    case ParcelType::MULTICOMPARTMENT:
        return tr("Multicompartment");
    default:
        return tr("Other");
    }
}
