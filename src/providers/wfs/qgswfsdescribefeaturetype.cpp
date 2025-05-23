/***************************************************************************
    qgswfsdescribefeaturetype.cpp
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsdescribefeaturetype.h"
#include "moc_qgswfsdescribefeaturetype.cpp"
#include <QUrlQuery>

QgsWFSDescribeFeatureType::QgsWFSDescribeFeatureType( QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
{
}

bool QgsWFSDescribeFeatureType::requestFeatureType( const QString &WFSVersion, const QString &typeName, const QgsWfsCapabilities &caps )
{
  QUrl url( mUri.requestUrl( QStringLiteral( "DescribeFeatureType" ), mUri.httpMethod() ) );

  switch ( mUri.httpMethod() )
  {
    case Qgis::HttpMethod::Get:
    {
      QUrlQuery query( url );
      query.addQueryItem( QStringLiteral( "VERSION" ), WFSVersion );

      const QString namespaceValue( caps.getNamespaceParameterValue( WFSVersion, typeName ) );

      if ( WFSVersion.startsWith( QLatin1String( "2.0" ) ) )
      {
        query.addQueryItem( QStringLiteral( "TYPENAMES" ), typeName );
        if ( !namespaceValue.isEmpty() )
        {
          query.addQueryItem( QStringLiteral( "NAMESPACES" ), namespaceValue );
        }
      }

      // Always add singular form for broken servers
      // See: https://github.com/qgis/QGIS/issues/41087
      query.addQueryItem( QStringLiteral( "TYPENAME" ), typeName );
      if ( !namespaceValue.isEmpty() )
      {
        query.addQueryItem( QStringLiteral( "NAMESPACE" ), namespaceValue );
      }

      url.setQuery( query );
      return sendGET( url, QString(), true, false );
    }

    case Qgis::HttpMethod::Post:
    {
      QDomDocument postDocument = createPostDocument();
      QDomElement describeFeatureTypeElement = createRootPostElement( caps, WFSVersion, postDocument, QStringLiteral( "wfs:DescribeFeatureType" ), { typeName } );

      QDomElement typeNameElement = postDocument.createElement( QStringLiteral( "wfs:TypeName" ) );
      typeNameElement.appendChild( postDocument.createTextNode( typeName ) );
      describeFeatureTypeElement.appendChild( typeNameElement );

      return sendPOST( url, QStringLiteral( "application/xml; charset=utf-8" ), postDocument.toByteArray(), true, { QNetworkReply::RawHeaderPair { "Accept", "application/xml" } } );
    }

    case Qgis::HttpMethod::Head:
    case Qgis::HttpMethod::Put:
    case Qgis::HttpMethod::Delete:
      // not supported, impossible to hit
      break;
  }
  return false;
}

QString QgsWFSDescribeFeatureType::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature type failed: %1" ).arg( reason );
}
